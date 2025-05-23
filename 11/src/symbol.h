#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>
#include "global.h"

int lookup(char s[]);
int insert(char s[], int tok, int value);

/** scoped symbol table */

typedef enum SymbolCategory {
  SC_NONE,
  SC_ClASS,
  SC_SUBROUTINE,
} SymbolCategory;

typedef enum VariableKind {
  VK_STATIC,
  VK_FIELD,
  VK_ARG,
  VK_VAR,
} VariableKind;

#define SYMBOL_TABLE_SIZE 100

typedef struct ScopedSymbolTableEntry
{
  char *name;
  char *type;
  VariableKind kind;
  int position;
} ScopedSymbolTableEntry;

/** we will clear the table by resetting the stats,
 * rather than actually clearing memory */
typedef struct ScopedSymbolTable
{
  char *name;

  // position for each kind
  uint8_t nextStatic;
  uint8_t nextField;
  uint8_t nextArg;
  uint8_t nextVar;

  // entries
  uint8_t currentIndex;
  uint8_t length;
  ScopedSymbolTableEntry *entries;
} ScopedSymbolTable;

/** resets the scoped symbol table */
void initSymbolTable(char *name, ScopedSymbolTable *table);
void cleanSymbolTable(ScopedSymbolTable *table);

/** accepts global symbols and defines scoped symbols */
void defineScopedSymbol(ScopedSymbolTable *table, char *name, char *type, VariableKind kind);
int indexOf(ScopedSymbolTable *table, char *name);
void debugTable(char *context, ScopedSymbolTable *table);
int varCount(ScopedSymbolTable *table, VariableKind kind);

ScopedSymbolTableEntry *getIndexFromGlobalTables(char *identifier);

// two global symbol tables
extern ScopedSymbolTable classSymbolTable;
extern ScopedSymbolTable subroutineSymbolTable;

#endif