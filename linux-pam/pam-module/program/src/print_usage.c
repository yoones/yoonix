#include <stdio.h>
#include <stdlib.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>
#include <grp.h>

void
print_usage(char *prog_name)
{
  fprintf(stderr, "Usage: %s [-h | --help | --change_groups]\n", prog_name);
  fprintf(stderr, "  -h, --help      : display this usage\n");
  fprintf(stderr, "  --change-groups : call pam_setcred(), which won't be called otherwise.\n");
  fprintf(stderr, "\n--change-groups needs this program to be own by root and setuid.\n");
  fprintf(stderr, "To do so, do the following:\n");
  fprintf(stderr, "$ sudo chown root %s\n", prog_name);
  fprintf(stderr, "$ sudo chmod +s %s\n", prog_name);
}
