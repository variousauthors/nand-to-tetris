#ifndef EMITTER_VM_H
#define EMITTER_VM_H

#include "symbol.h"
#include <stdio.h>
#include <stdbool.h>

void initEmitterVM(FILE *out);

typedef enum EM_OP
{
  EM_MUL,
  EM_ADD,
  EM_SUB,
  EM_DIV,
  EM_AND,
  EM_OR,
  EM_LT,
  EM_GT,
  EM_EQ,
  EM_NEG,
  EM_NOT,
} EmitterOp;

void emitOperation(EmitterOp op);
void emitTermBool(bool value);
void emitTermInteger(int n);
void emitMethodCall(char *objectName, char *id2, int argc);
void emitVoidReturnValue();
void emitReturn();
void emitFunctionDeclaration(char *className, char *functionName, int argc);
void emitVariableAssignment(ScopedSymbolTableEntry *entry);
void emitVariableReference(ScopedSymbolTableEntry *entry);
void emitVoidFunctionCleanup();
void emitLabel(char *label);
void emitWhileLoopLoop(char *loop);
void emitWhileLoopTest(char *done);
void emitIfSkip(char *done);
void emitIfCondition(char *elseBlock);
#endif