#ifndef PAM_SM_AUTH
# define PAM_SM_AUTH
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <unistd.h>
#include "discreet_display.h"

static int
_add_user_to_group(const char *group_name,
		   int *nb_groups,
		   gid_t *list)
{
  struct group *grp;
  int i;
    
  /* read group entry */
  grp = getgrnam(group_name);
  if (grp == NULL)
    return (PAM_CRED_ERR);

  /* check if user already is in this group */
  for (i = 0; i < *nb_groups; i++)
    {
      if (list[i] == grp->gr_gid)
	return (PAM_IGNORE);
    }
  
  /* add user to group */
  list[*nb_groups] = grp->gr_gid;
  (*nb_groups)++;
  return (PAM_SUCCESS);
}

static int
_del_user_from_group(const char *group_name,
		     int *nb_groups,
		     gid_t *list)
{
  struct group *grp;
  int i;
    
  /* read group entry */
  grp = getgrnam(group_name);
  if (grp == NULL)
    return (PAM_CRED_ERR);

  /* check if user already is in this group */
  for (i = 0; i < *nb_groups; i++)
    {
      if (list[i] == grp->gr_gid)
	{
	  /* remove user from group */
	  list[i] = list[(*nb_groups) - 1];
	  (*nb_groups)--;
	  return (PAM_SUCCESS);
	}
    }

  /* user not in group */
  return (PAM_IGNORE);
}

static int
_change_groups(int argc, const char **argv)
{
  int nb_groups;
  int max_nb_groups;
  gid_t *list;
  const char *grpname;
  int ret;
  int i;
  
  /* get number of groups */
  nb_groups = getgroups(0, NULL);
  if (nb_groups == -1)
    {
      discreet_perror("getgroups");
      return (PAM_CRED_ERR);
    }

  /* allocate enough memory for final list of groups */
  max_nb_groups = nb_groups + argc;
  list = calloc(max_nb_groups, sizeof(gid_t));
  if (!list)
    {
      discreet_perror("calloc");
      return (PAM_CRED_ERR);
    }

  /* read current groups list */
  if (getgroups(nb_groups, list) == -1)
    {
      discreet_perror("getgroups");
      return (PAM_CRED_UNAVAIL);
    }

  /* apply changes */
  for (i = 0; i < argc; i++)
    {
      if (!strncmp(argv[i], "addgrp=", 7))
	{
	  grpname = argv[i] + 7;
	  if (*grpname == '\0')
	    goto bad_arg;
	  if (_add_user_to_group(grpname, &nb_groups, list) == PAM_CRED_ERR)
	    goto group_change_failure;
	}
      else if (!strncmp(argv[i], "delgrp=", 7))
	{
	  grpname = argv[i] + 7;
	  if (*grpname == '\0')
	    goto bad_arg;
	  if (_del_user_from_group(grpname, &nb_groups, list) == PAM_CRED_ERR)
	    goto group_change_failure;
	}
      else
	goto bad_arg;
    }

  ret = PAM_SUCCESS;
  
  /* apply new groups list */
  if (setgroups(nb_groups, list) != 0)
    {
      discreet_perror("setgroups");
      ret = PAM_CRED_ERR;
    }
  free(list);
  return (ret);
  
 bad_arg:
  discreet_fprintf(stderr, "Error: bad argument given\n");
  free(list);
  return (PAM_CRED_ERR);

 group_change_failure:
  free(list);
  return (PAM_CRED_ERR);
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh,
	       int flags,
	       int argc,
	       const char **argv)
{
  const char *username;
  int ret;
  
  /* initialize display functions */
  initialize_discreet_functions(flags);

  /* get username to make sure pam_sm_authenticate() was called first */
  ret = pam_get_data(pamh, "USERNAME", (void *)&username);
  if (ret != PAM_SUCCESS)
    {
      discreet_fprintf(stderr, "pam_get_data: %s\n", pam_strerror(pamh, ret));
      return (PAM_USER_UNKNOWN);
    }

  /* apply changes */
  ret = _change_groups(argc, argv);
  
  return (ret);
}
