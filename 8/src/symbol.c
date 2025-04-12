#include "error.h"
#include "global.h"
#include <string.h>

#define STRMAX 32000
#define SYMMAX 1600

char lexemes[STRMAX];
int lastchar = -1;
struct entry symtable[SYMMAX];
int lastentry = 0;

int lookup(char s[])
{
  int p;
  for (p = lastentry; p > 0; p--)
  {
    if (strcmp(symtable[p].lexptr, s) == 0)
    {
      return p;
    }
  }

  return 0;
}

int insert(char s[], int tok, int value)
{
  int len;
  len = strlen(s);
  if (lastentry + 1 >= SYMMAX)
  {
    error("symbol table full");
  }
  if (lastchar + len + 1 >= STRMAX)
  {
    error("lexemes array is full");
  }

  lastentry = lastentry + 1;
  symtable[lastentry].token = tok;
  symtable[lastentry].value = value;
  symtable[lastentry].lexptr = &lexemes[lastchar + 1];
  lastchar = lastchar + len + 1;
  strcpy(symtable[lastentry].lexptr, s);

  return lastentry;
}
