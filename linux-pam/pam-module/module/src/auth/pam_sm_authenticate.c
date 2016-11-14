#ifndef PAM_SM_AUTH
# define PAM_SM_AUTH
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <errno.h>
#include "discreet_display.h"

static int
_prompt_user_for_auth_info(pam_handle_t *pamh,
			   const char **username,
			   const char **password)
{
  int ret;
  
  /* prompt user for username */
  ret = pam_get_user(pamh, username, "login: ");
  if (ret != PAM_SUCCESS || *username == NULL)
    {
      discreet_fprintf(stderr, "Error: failed to obtain user's username\n");
      return (ret);
    }
  
  /* prompt user for password */
  ret = pam_get_authtok(pamh, PAM_AUTHTOK, password, "password: ");
  if (ret != PAM_SUCCESS || *password == NULL)
    {
      discreet_fprintf(stderr, "Error: failed to obtain user's password\n");
      return (ret);
    }

  return (PAM_SUCCESS);
}

static int
_get_passwd_entry(const char *username, struct passwd **pw)
{
  *pw = getpwnam(username);
  if (*pw == NULL)
    {
      switch (errno)
	{
	case (EINTR):
	case (EIO):
	case (EMFILE):
	case (ENFILE):
	case (ENOMEM):
	case (ERANGE):
	  discreet_perror("getpwnam");
	  return (PAM_AUTHINFO_UNAVAIL);
	default:
	  discreet_fprintf(stderr, "Error: bad username/password\n");
	  return (PAM_USER_UNKNOWN);
	}
    }
  else
    return (PAM_SUCCESS);
}

static void
_cleanup(pam_handle_t *pamh, void *data, int error_status)
{
  (void)pamh;
  (void)error_status;
  free(data);
}

/*
** To verify the password of a user, we need to read /etc/shadow.
** To do so requires either root privileges or shadow group membership.
** Since pam modules inherit the calling pam-aware program's privileges,
** they can't just call getspnam(). They need to execute a setuid
** program that will be able to read /etc/shadow for them.
**
** man 8 unix_chkpwd
** unix_chkpwd - Helper binary that verifies the password of the current user
**
** _verify_password() forks and executes /sbin/unix_chkpwd. Normally, I
** would've wrote my own setuid program, but there's no benefit for this
** tutorial to do so, so let's just use pam_unix's helper here.
*/
static int
_verify_password(const char *username, const char *password, int flags)
{
  pid_t pid;
  char *argv[] = {
    "/sbin/unix_chkpwd",
    (char *)username,
    (flags & PAM_DISALLOW_NULL_AUTHTOK ? "nonull" : "nullok"),
    NULL
  };
  int pipefd[2];
  int status;
  
  if (pipe(pipefd) != 0)
    {
      discreet_perror("pipe");
      return (PAM_AUTHINFO_UNAVAIL);
    }
  pid = fork();
  if (pid == -1)
    {
      discreet_perror("fork");
      close(pipefd[0]);
      close(pipefd[1]);
      return (PAM_AUTHINFO_UNAVAIL);
    }
  if (pid != 0)
    {
      close(pipefd[0]);
      write(pipefd[1], password, strlen(password));
      write(pipefd[1], "", 1);
      close(pipefd[1]);
      if (wait(&status) == -1
	  || WIFEXITED(status) == 0)
	{
	  discreet_perror("wait");
	  return (PAM_AUTHINFO_UNAVAIL);
	}
      return (WEXITSTATUS(status));
    }
  else
    {
      close(pipefd[1]);
      if (dup2(pipefd[0], 0) != 0)
	{
	  discreet_perror("dup2");
	  exit(PAM_AUTHINFO_UNAVAIL);
	}
      execve(argv[0], argv, NULL);
      discreet_perror("execve");
      close(pipefd[0]);
      exit(PAM_AUTHINFO_UNAVAIL);
    }
}

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh,
		    int flags,
		    int argc,
		    const char **argv)
{
  const char *username;
  const char *password;
  struct passwd *pw;
  int ret;

  (void)argc;
  (void)argv;
  
  /* initialize display functions */
  initialize_discreet_functions(flags);
  
  /* prompt user for username and password */
  ret = _prompt_user_for_auth_info(pamh, &username, &password);
  if (ret != PAM_SUCCESS)
    return (ret);

  /* read /etc/passwd */
  ret = _get_passwd_entry(username, &pw);
  if (ret != PAM_SUCCESS)
    return (ret);

  /* verify username/password */
  if (_verify_password(username, password, flags) != PAM_SUCCESS)
    {
      discreet_fprintf(stderr, "Error: bad username/passwd\n");
      return (PAM_AUTH_ERR);
    }

  /* user successfully authenticated. yey! */

  /* now let's store user's username for pam_sm_setcred() to later do its job */
  username = strdup(username);
  if (username == NULL)
    {
      discreet_perror("strdup");
      return (PAM_AUTH_ERR);
    }
  if (pam_set_data(pamh, "USERNAME", (char *)username, _cleanup) != PAM_SUCCESS)
    {
      discreet_fprintf(stderr, "pam_set_data() failed (probably due to lack of memory)\n");
      return (PAM_AUTH_ERR);
    }

  /* done. */
  return (PAM_SUCCESS);
}
