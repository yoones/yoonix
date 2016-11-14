#ifndef PAM_SM_PASSWORD
# define PAM_SM_PASSWORD
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

int
pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return (PAM_IGNORE);
}
