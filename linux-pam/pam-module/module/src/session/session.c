#ifndef PAM_SM_SESSION
# define PAM_SM_SESSION
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

int
pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return (PAM_IGNORE);
}

int
pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return (PAM_IGNORE);
}
