#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

int
converse(int n, const struct pam_message **msg,
	 struct pam_response **resp, void *data)
{
  return (PAM_CONV_ERR);
}
