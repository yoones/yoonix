#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sqlite3.h>

#define DATABASE_FILEPATH "./db.sqlite"

int
callback(void *data,
	 int argc,
	 char **argv,
	 char **colums_names)
{
  int i;

  (void)data;
  printf("row found:\n");

  /* print content of each parameter recieved */
  printf("  params:\n");
  /* 1. display number of columns */
  printf("    argc = %d\n", argc);
  /* 2. display columns names */
  for (i = 0; i < argc; i++)
    printf("    colums_names[%d] = (%s)\n", i, colums_names[i]);
  /* 3. display columns values */
  for (i = 0; i < argc; i++)
    printf("    argv[%d] = (%s)\n", i, argv[i]);
  printf("  row:\n");
  /* 4. display row in a different fashion */
  for (i = 0; i < argc; i++)
    printf("    %s = (%s)\n", colums_names[i], argv[i]);
  printf("\n");
  return (0);
}

int
main()
{
  sqlite3 *db;
  char *query = "select * from tasks;";
  int ret;
  char *err_msg;
  
  /* open database */
  if (sqlite3_open(DATABASE_FILEPATH, &db) != 0)
    {
      fprintf(stderr, "Error: failed to open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return (EXIT_FAILURE);
    }
  /* execute sql query */
  ret = sqlite3_exec(db, query, callback, NULL, &err_msg);
  if (ret != SQLITE_OK)
    {
      fprintf(stderr, "Error: %s\n", err_msg);
      sqlite3_free(err_msg);
      sqlite3_close(db);
      return (EXIT_FAILURE);
    }
  /* close database */
  sqlite3_close(db);
  return (EXIT_SUCCESS);
}
