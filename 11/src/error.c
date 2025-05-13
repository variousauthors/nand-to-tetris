#include "global.h"
#include <stdbool.h>
#include "error.h"

bool error(const char *m)
{
  fprintf(stderr, "%s line %d: %s\n", currentFile, lineno, m);
  exit(1);
  return false;
}

void say(char *message)
{
  fprintf(stderr, "%s\n", message);
}

void problem(int n)
{
  fprintf(stderr, "%d\n", n);
}
