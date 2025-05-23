
/** register D will get the value baseaddr + offset for the given segment */
#include <stdio.h>
#include <stdbool.h>
#include "emitterXML.h"
#include "error.h"
#include <sys/syslimits.h>
#include <_stdio.h>

#define TEMP_BASEADDR 5

static int indent = 0;
static const int MAX_INDENT = 128;
static const int SPACES = 2;

static FILE *outfile;

/** returns a size appropriate for indentation whitespace */
static int indentSize(int indent)
{
  return indent * SPACES + 1;
}

static void makeIndentation(char *indentation)
{
  int i = 0;
  for (; i < indent * SPACES && i < MAX_INDENT; i++)
  {
    indentation[i] = ' ';
  }

  // terminate after the last char we actually wrote
  indentation[i] = '\0';
}

void initEmitterXML(FILE *out)
{
  outfile = out;
}

void emitXMLPrimitive(char *tag, char *value)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%s<%s> %s </%s>\n", indentation, tag, value, tag);
}

void emitXMLPrimitiveInteger(char *tag, int value)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%s<%s> %d </%s>\n", indentation, tag, value, tag);
}

void emitXMLOpenTag(char *tag)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%s<%s>\n", indentation, tag);
  indent++;
}

void emitXMLCloseTag(char *tag)
{
  indent--;

  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%s</%s>\n", indentation, tag);
}

void emitXMLSelfClosingTag(char *tag)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%s<%s/>\n", indentation, tag);
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

void emitIdentifierClass(char *identifier, char *mode)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  fprintf(outfile, "%s<identifier category=\"class\" mode=\"%s\"> %s </identifier>\n", indentation, mode, identifier);
}

void emitIdentifierSubroutine(char *identifier, char *mode)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  // output "declaration" as opposed to "reference"
  fprintf(outfile, "%s<identifier category=\"subroutine\" mode=\"%s\"> %s </identifier>\n", indentation, mode, identifier);
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

void emitVariableDefinitionXML(char *identifier)
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
    fprintf(outfile, "%s<identifier category=\"%s\" position=\"%d\" mode=\"%s\"> %s </identifier>\n", indentation, getCategory(entry->kind), entry->position, "declaration", entry->name);
  }
}

void emitVariableReferenceXML(char *identifier)
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
    fprintf(outfile, "%s<identifier category=\"%s\" position=\"%d\" mode=\"%s\"> %s </identifier>\n", indentation, getCategory(entry->kind), entry->position, "reference", entry->name);
  }
}