#include <stdio.h>
#include <stdlib.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>
#include <unistd.h>

#define PRINT_STEP() printf("\nStep: %s\n\n", __FUNCTION__)

void
print_usage(char *prog_name);

int
print_current_groups();

static struct pam_conv conv = {
  misc_conv,
  NULL
};

static void
_check_args(int argc, char **argv, int *change_groups)
{
  *change_groups = 0;
  if (argc > 1)
    {
      if (argc != 2)
	{
	  print_usage(argv[0]);
	  exit(EXIT_FAILURE);
	}
      else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
	{
	  print_usage(argv[0]);
	  exit(EXIT_SUCCESS);
	}
      else if (!strcmp(argv[1], "--change-groups"))
	*change_groups = 1;
      else
	{
	  print_usage(argv[0]);
	  exit(EXIT_FAILURE);
	}
    }
}

static int
_start_transaction_and_authenticate_user(pam_handle_t **pamh)
{
  int ret;

  PRINT_STEP();

  if ((ret = pam_start("dumb_test", NULL, &conv, pamh)) != PAM_SUCCESS)
    {
      fprintf(stderr, "Failed to start transaction\n");
      return (ret);
    }
  if ((ret = pam_authenticate(*pamh, 0)) != PAM_SUCCESS)
    {
      fprintf(stderr, "Error: failed to authenticate user\n");
      pam_end(*pamh, ret);
      return (ret);
    }
  printf("pam_authenticate() succeeded!\n");
  return (ret);
}

static int
_establish_credentials(pam_handle_t **pamh)
{
  int ret;

  PRINT_STEP();

  if ((ret = pam_setcred(*pamh, PAM_ESTABLISH_CRED)) != PAM_SUCCESS)
    {
      fprintf(stderr, "Error: failed to set credentials\n");
      pam_end(*pamh, ret);
      return (EXIT_FAILURE);
    }
  printf("pam_setcred(PAM_ESTABLISH_CRED) succeeded!\n");

  return (EXIT_SUCCESS);
}

/* PAM_DELETE_CRED isn't implemented in my pam module yet */
/* static int */
/* _delete_credentials(pam_handle_t **pamh) */
/* { */
/*   int ret; */

/*   PRINT_STEP(); */

/*   ret = pam_setcred(pamh, PAM_DELETE_CRED); */
/*   if (ret == PAM_SUCCESS) */
/*     printf("pam_setcred(PAM_DELETE_CRED) succeeded!\n"); */
/*   else */
/*     pam_end(pamh, ret); */
/*   return (ret); */
/* } */

int
main(int argc, char **argv)
{
  pam_handle_t *pamh = NULL;
  int ret;
  int change_groups;
  
  _check_args(argc, argv, &change_groups);

  ret = _start_transaction_and_authenticate_user(&pamh);
  if (ret != PAM_SUCCESS)
    return (EXIT_FAILURE);

  /* stop here if "--change-groups" option is not set */
  if (change_groups == 0)
    {
      pam_end(pamh, ret);
      return (EXIT_SUCCESS);
    }

  if (print_current_groups() != 0)
    return (EXIT_FAILURE);

  ret = _establish_credentials(&pamh);
  if (ret != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (print_current_groups() != 0)
    return (EXIT_FAILURE);

  /* PAM_DELETE_CRED isn't implemented in my pam module yet */

  /* ret = _delete_credentials(&pamh); */
  /* if (ret != EXIT_SUCCESS) */
  /*   return (EXIT_FAILURE); */

  /* if (print_current_groups() != 0) */
  /*   return (EXIT_FAILURE); */

  ret = pam_end(pamh, ret);
  return (ret == PAM_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE);
}
