#include <stdio.h>
#include <errno.h>
#include <security/pam_modules.h>
#include <stdarg.h>

int (*discreet_fprintf)(FILE *stream, const char *format, ...);
void (*discreet_perror)(const char *s);

static int do_nothing_fprintf(FILE *stream, const char *format, ...)
{
  (void)stream;
  (void)format;
  return (0);
}

static void do_nothing_perror(const char *s)
{
  (void)s;
}

void
initialize_discreet_functions(int flags)
{
  if ((flags & PAM_SILENT) == 0)
    {
      discreet_fprintf = fprintf;
      discreet_perror = perror;
    }
  else
    {
      discreet_fprintf = do_nothing_fprintf;
      discreet_perror = do_nothing_perror;
    }
}
