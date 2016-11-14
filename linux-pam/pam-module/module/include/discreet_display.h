#ifndef DISCREET_DISPLAY_H_
# define DISCREET_DISPLAY_H_

extern int (*discreet_fprintf)(FILE *stream, const char *format, ...);
extern void (*discreet_perror)(const char *s);

void
initialize_discreet_functions(int flags);

#endif
