#include <stdio.h>
#include "emitterVM.h"
#include "symbol.h"

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

void initEmitterVM(FILE *out)
{
  outfile = out;
}

char *getSegment(VariableKind kind)
{
  switch (kind)
  {
  case VK_STATIC:
    return "static";
  case VK_FIELD:
    return "this";
  case VK_ARG:
    return "argument";
  case VK_VAR:
    return "local";
  default:
    break;
  }
}

typedef enum VMConstants
{
  EM_TRUE,
  EM_INTEGER,
} VMConstants;

void emitTermBool(bool value)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  if (value)
  {
    // TRUE is 1111111111111111 (AKA -1)
    fprintf(outfile, "%spush constant 1\n", indentation);
    fprintf(outfile, "%sneg\n", indentation);
  }
  else
  {
    fprintf(outfile, "%spush constant 0\n", indentation);
  }
}

void emitVariableReference(ScopedSymbolTableEntry *entry)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%spush %s %d\n", indentation, getSegment(entry->kind), entry->position);
}

void emitVariableAssignment(ScopedSymbolTableEntry *entry)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%spop %s %d\n", indentation, getSegment(entry->kind), entry->position);
}

void emitVoidFunctionCleanup()
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%spop temp 0\n", indentation);
}

void emitFunctionDeclaration(char *className, char *functionName, int argc)
{
  indent++;
  fprintf(outfile, "function %s.%s %d\n", className, functionName, argc);
}

void emitReturn()
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);
  fprintf(outfile, "%sreturn\n", indentation);
  indent--;
}

void emitVoidReturnValue()
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%spush constant 0\n", indentation);
}

void emitMethodCall(char *objectName, char *methodName, int argc)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%scall %s.%s %d\n", indentation, objectName, methodName, argc);
}

void emitLabel(char *label)
{
  fprintf(outfile, "label %s\n", label);
}

void emitWhileLoopTest(char *done)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%sneg\n", indentation);
  fprintf(outfile, "%sif-goto %s\n", indentation, done);
}

void emitWhileLoopLoop(char *loop)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%sgoto %s\n", indentation, loop);
}

void emitTermInteger(int n)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%spush constant %d\n", indentation, n);
}

void emitOperation(char op)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  switch (op)
  {
  case '*':
  {
    fprintf(outfile, "%scall Math.multiply 2\n", indentation);
    break;
  }
  case '+':
  {
    fprintf(outfile, "%sadd\n", indentation);
    break;
  }
  default:
    break;
  }
}
