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

typedef struct ScopedSymbolTableEntry
{
  Entry *name;
  Entry *type;
  VariableKind kind;
  int position;
} ScopedSymbolTableEntry;

/** we will clear the table by resetting the stats,
 * rather than actually clearing memory */
typedef struct ScopedSymbolTable
{
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
void startSubroutine(ScopedSymbolTable *table);

/** accepts global symbols and defines scoped symbols */
void defineScopedSymbol(ScopedSymbolTable *table, Entry *name, Entry *type, VariableKind kind);
int indexOf(ScopedSymbolTable *table, char *name);

ScopedSymbolTableEntry *getIndexFromGlobalTables(char *identifier);

// two global symbol tables
extern ScopedSymbolTable classSymbolTable;
extern ScopedSymbolTable subroutineSymbolTable;

#endif