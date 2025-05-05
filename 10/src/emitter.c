
/** register D will get the value baseaddr + offset for the given segment */
#include <stdio.h>
#include <stdbool.h>
#include "emitter.h"
#include "error.h"
#include <sys/syslimits.h>
#include <_stdio.h>

#define TEMP_BASEADDR 5

int indent = 0;
int MAX_INDENT = 128;
int SPACES = 2;

/** returns a size appropriate for indentation whitespace */
int indentSize(int indent)
{
  return indent * SPACES + 1;
}

void makeIndentation(char *indentation)
{
  int i = 0;
  for (; i < indent * SPACES && i < MAX_INDENT; i++)
  {
    indentation[i] = ' ';
  }

  // terminate after the last char we actually wrote
  indentation[i] = '\0';
}

void emitXMLPrimitive(char *tag, char *value)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  printf("%s<%s> %s </%s>\n", indentation, tag, value, tag);
}

void emitXMLPrimitiveInteger(char *tag, int value)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  printf("%s<%s> %d </%s>\n", indentation, tag, value, tag);
}

void emitXMLOpenTag(char *tag)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  printf("%s<%s>\n", indentation, tag);
  indent++;
}

void emitXMLCloseTag(char *tag)
{
  indent--;

  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  printf("%s</%s>\n", indentation, tag);
}

void emitXMLSelfClosingTag(char *tag)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  printf("%s<%s/>\n", indentation, tag);
}

void emitSubroutineBodyOpen()
{
  emitXMLOpenTag("subroutineBody");
  emitSymbol("{");
}

void emitSubroutineBodyClose()
{
  emitXMLCloseTag("subroutineBody");
}

void emitClassOpen()
{
  emitXMLOpenTag("class");
}

void emitClassClose()
{
  emitXMLCloseTag("class");
}

void emitKeyword(char *keyword)
{
  emitXMLPrimitive("keyword", keyword);
}

void emitSymbol(char *symbol)
{
  if (symbol[1] == '\0')
  {
    // this might need to be escaped
    char initial = symbol[0];

    switch (initial)
    {
    case '&':
    {
      emitXMLPrimitive("symbol", "&amp;");
      return;
    }
    case '<':
    {
      emitXMLPrimitive("symbol", "&lt;");
      return;
    }
    case '>':
    {
      emitXMLPrimitive("symbol", "&gt;");
      return;
    }
    default:
      break;
    }
  }

  emitXMLPrimitive("symbol", symbol);
}

void emitIdentifier(char *identifier)
{
  emitXMLPrimitive("identifier", identifier);
}

void emitIdentifierClass(char *identifier, char *mode)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  printf("%s<identifier category=\"class\" mode=\"%s\"> %s </identifier>\n", indentation, mode, identifier);
}

void emitIdentifierSubroutine(char *identifier, char *mode)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  printf("%s<identifier category=\"subroutine\" mode=\"%s\"> %s </identifier>\n", indentation, mode, identifier);
}

char *getCategory(VariableKind kind)
{
  switch (kind)
  {
  case VK_STATIC:
    return "static";
  case VK_FIELD:
    return "field";
  case VK_ARG:
    return "argument";
  case VK_VAR:
    return "var";
  default:
    break;
  }
}

ScopedSymbolTableEntry *getIndexFromGlobalTables(char *identifier)
{
  // first check the subroutine table
  int index;

  index = indexOf(&subroutineSymbolTable, identifier);

  if (index >= 0)
  {
    return &subroutineSymbolTable.entries[index];
  }

  index = indexOf(&classSymbolTable, identifier);

  if (index >= 0)
  {
    return &classSymbolTable.entries[index];
  }

  return 0;
}

void emitIdentifierSpecial(char *identifier, char *mode)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // first check the subroutine table
  ScopedSymbolTableEntry *entry = getIndexFromGlobalTables(identifier);

  if (entry == 0)
  {
    // it might be a class or subroutine
    int index = lookup(identifier);

    fprintf(stderr, "encountered unidentified symbol %s\n", identifier);
    error("while emitting identifier definition");
  }
  else
  {
    printf("%s<identifier category=\"%s\" position=\"%d\" mode=\"%s\"> %s </identifier>\n", indentation, getCategory(entry->kind), entry->position, mode, entry->name->lexptr);
  }
}