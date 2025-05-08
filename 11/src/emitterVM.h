#ifndef EMITTER_VM_H
#define EMITTER_VM_H

#include <stdio.h>

void initEmitterVM(FILE *out);

void emitOperation(char op);
void emitConstant(int n);
void emitMethodCall(char *objectName, char *id2, int argc);
void emitVoidReturnValue();
void emitReturn();
void emitFunctionDeclaration(char *className, char *functionName, int argc);
void emitVoidFunctionCleanup();

#endif