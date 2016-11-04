#include <stdio.h>
#include <stdlib.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>

static struct pam_conv conv = {
  misc_conv,
  NULL
};

int
authenticate_user(char **username)
{
  pam_handle_t *pamh = NULL;
  int ret;
  const void *_username;

  if ((ret = pam_start("login", NULL, &conv, &pamh)) != PAM_SUCCESS
      || (ret = pam_authenticate(pamh, 0)) != PAM_SUCCESS
      || (ret = pam_acct_mgmt(pamh, 0)) != PAM_SUCCESS
      || (ret = pam_get_item(pamh, PAM_USER, &_username)) != PAM_SUCCESS
      || _username == NULL)
    {
      fprintf(stderr, "Error: failed to authenticate user\n");
      pam_end(pamh, ret);
      return (1);
    }
  *username = strdup(_username);
  ret = pam_end(pamh, ret);
  if (ret != PAM_SUCCESS)
    {
      fprintf(stderr, "Error: failed to retrieve username\n");
      pam_end(pamh, ret);
      return (1);
    }
  return (0);
}

int
main()
{
  char *username;

  if (authenticate_user(&username) != 0)
    return (EXIT_FAILURE);
  printf ("Hi %s, happy to see you!\n", username);
  free(username);
  return (EXIT_SUCCESS);
}
