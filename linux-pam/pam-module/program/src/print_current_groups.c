#include <stdio.h>
#include <stdlib.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>
#include <grp.h>

int
print_current_groups()
{
  int nb_groups;
  gid_t *list;
  int i;
  struct group *grp;
  
  nb_groups = getgroups(0, NULL);
  printf("%d group(s) found:\n", nb_groups);
  list = calloc(nb_groups, sizeof(gid_t));
  if (!list)
    {
      perror("calloc");
      return (1);
    }
  getgroups(nb_groups, list);
  for (i = 0; i < nb_groups; i++)
    {
      grp = getgrgid(list[i]);
      printf("  %s\n", grp->gr_name);
    }
  printf("\n");
  free(list);
  return (0);
}
