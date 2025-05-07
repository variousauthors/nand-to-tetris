#include "error.h"
#include "symbol.h"
#include <string.h>

/** this symbol table is used by the tokenizer to store
 * identifiers... it is invoked when we encounter a token
 * and doesn't really fit for the class symbol table or
 * subroutine symbol table use case... so lets
 * implement another symbol table that can do the job
 */

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

/** == Scoped Symbol Table ==  */
void startSubroutine(ScopedSymbolTable *table) {
  table->currentIndex = 0;
  table->length = 0;
  table->nextStatic = 0;
  table->nextField = 0;
  table->nextArg = 0;
  table->nextVar = 0;
}

void defineScopedSymbol(ScopedSymbolTable *table, Entry *name, Entry *type, VariableKind kind) {
  // TODO bounds checking on the underlying array

  table->entries[table->currentIndex].name = name;
  table->entries[table->currentIndex].type = type;
  table->entries[table->currentIndex].kind = kind;

  switch (kind)
  {
  case VK_STATIC: { 
    table->entries[table->currentIndex].position = table->nextStatic;
    table->nextStatic++;
    break;
  }
  case VK_FIELD: { 
    table->entries[table->currentIndex].position = table->nextField;
    table->nextField++;
    break;
  }
  case VK_ARG: { 
    table->entries[table->currentIndex].position = table->nextArg;
    table->nextArg++;
    break;
  }
  case VK_VAR: { 
    table->entries[table->currentIndex].position = table->nextVar;
    table->nextVar++;
    break;
  }
  default:
    fprintf(stderr, "tried to add symbol of kind %d\n", kind);
    error("while defining a scoped symbol");
    break;
  }

  table->currentIndex++;
  table->length++;
}

int indexOf(ScopedSymbolTable *table, char *name) {
  for (int i = 0; i < table->length; i++) {
    if (strcmp(name, table->entries[i].name->lexptr) == 0) {
      return i;
    }
  }

  return -1;
}