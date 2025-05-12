#include <stdio.h>
#include <string.h>
#include "emitterVM.h"
#include "error.h"
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

void emitThisReference()
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%spush pointer 0\n", indentation);
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

void emitInstanceForCall(ScopedSymbolTableEntry *entry)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  switch (entry->kind)
  {
  case VK_FIELD:
  {
    fprintf(outfile, "%spush this %d\n", indentation, entry->position);
    break;
  }
  case VK_STATIC:
  {
    fprintf(outfile, "%spush static %d\n", indentation, entry->position);
    break;
  }
  case VK_ARG:
  {
    fprintf(outfile, "%spush argument %d\n", indentation, entry->position);
    break;
  }
  case VK_VAR:
  {
    fprintf(outfile, "%spush local %d\n", indentation, entry->position);
    break;
  }
  default:
    error("failed to emit instance for call");
  }
}

void emitImplicitThis()
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%spush argument 0\n", indentation);
  fprintf(outfile, "%spop pointer 0\n", indentation);
}

void emitAllocateInstance(ScopedSymbolTable *table)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);
  int fieldsCount = varCount(table, VK_FIELD);

  // allocate n bytes where n is the number of fields on the class
  fprintf(outfile, "%spush constant %d\n", indentation, fieldsCount);
  fprintf(outfile, "%scall Memory.alloc 1\n", indentation);
  fprintf(outfile, "%spop pointer 0\n", indentation);
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

void emitFunctionCall(char *objectName, int argc)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%scall %s %d\n", indentation, objectName, argc);
}

void emitLabel(char *label)
{
  fprintf(outfile, "label %s\n", label);
}

void emitIfCondition(char *elseBlock)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%snot\n", indentation);
  fprintf(outfile, "%sif-goto %s\n", indentation, elseBlock);
}

void emitStringDefinition(char *str)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  size_t length = strlen(str);

  fprintf(outfile, "%spush String.new %zu\n", indentation, length);

  for (int i = 0; i < length; i++)
  {
    char ch = str[i];
    fprintf(outfile, "%spush constant %d\n", indentation, ch);
    fprintf(outfile, "%spush String.appendChar %d\n", indentation, 2);
  }
}

void emitIfSkip(char *done)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%sgoto %s\n", indentation, done);
}

void emitWhileLoopTest(char *done)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  fprintf(outfile, "%snot\n", indentation);
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

void emitOperation(EmitterOp op)
{
  char indentation[indentSize(indent)];
  makeIndentation(indentation);

  switch (op)
  {
  case EM_NOT:
  {
    fprintf(outfile, "%snot\n", indentation);
    break;
  }
  case EM_NEG:
  {
    fprintf(outfile, "%sneg\n", indentation);
    break;
  }
  case EM_MUL:
  {
    fprintf(outfile, "%scall Math.multiply 2\n", indentation);
    break;
  }
  case EM_ADD:
  {
    fprintf(outfile, "%sadd\n", indentation);
    break;
  }
  case EM_SUB:
  {
    fprintf(outfile, "%ssub\n", indentation);
    break;
  }
  case EM_DIV:
  {
    fprintf(outfile, "%sMath.divide\n", indentation);
    break;
  }
  case EM_AND:
  {
    fprintf(outfile, "%sand\n", indentation);
    break;
  }
  case EM_OR:
  {
    fprintf(outfile, "%sor\n", indentation);
    break;
  }
  case EM_LT:
  {
    fprintf(outfile, "%slt\n", indentation);
    break;
  }
  case EM_GT:
  {
    fprintf(outfile, "%sgt\n", indentation);
    break;
  }
  case EM_EQ:
  {
    fprintf(outfile, "%seq\n", indentation);
    break;
  }
  default:
    fprintf(stderr, "encountered unidentified operator %d\n", op);
    error("while emitting operations");
    break;
  }
}
