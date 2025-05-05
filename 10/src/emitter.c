
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

void emitIdentifierType(char *identifier)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  printf("%s<identifier category=\"type\" mode=\"reference\"> %s </identifier>\n", indentation, identifier);
}

void emitIdentifierClass(char *identifier)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  printf("%s<identifier category=\"class\" mode=\"declaration\"> %s </identifier>\n", indentation, identifier);
}

void emitIdentifierSubroutine(char *identifier)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  printf("%s<identifier category=\"subroutine\" mode=\"declaration\"> %s </identifier>\n", indentation, identifier);
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

void emitIdentifierDefinition(char *identifier, ScopedSymbolTable *table)
{
  int index = indexOf(table, identifier);

  if (index < 0)
  {
    fprintf(stderr, "could not find %s in scoped symbol table\n", identifier);
    error("while emitting identifier definition");
  }

  ScopedSymbolTableEntry entry = table->entries[index];

  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  printf("%s<identifier category=\"%s\" position=\"%d\" mode=\"declaration\"> %s </identifier>\n", indentation, getCategory(entry.kind), entry.position, entry.name->lexptr);
}

void emitIdentifierReference(char *identifier, ScopedSymbolTable *table)
{
  int index = indexOf(table, identifier);

  if (index < 0)
  {
    fprintf(stderr, "could not find %s in scoped symbol table\n", identifier);
    error("while emitting identifier definition");
  }

  ScopedSymbolTableEntry entry = table->entries[index];

  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  printf("%s<identifier category=\"%s\" position=\"%d\" mode=\"reference\"> %s </identifier>\n", indentation, getCategory(entry.kind), entry.position, entry.name->lexptr);
}
