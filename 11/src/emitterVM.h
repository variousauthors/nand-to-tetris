#ifndef EMITTER_VM_H
#define EMITTER_VM_H

#include "symbol.h"
#include <stdio.h>
#include <stdbool.h>

void initEmitterVM(FILE *out);

void emitOperation(char op);
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
#endif