#include "global.h"
#include <stdbool.h>
#include "error.h"

bool error(const char *m)
{
  fprintf(stderr, "line %d: %s\n", lineno, m);
  exit(1);
  return false;
}