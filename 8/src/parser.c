#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>

#include "buffer.h"
#include "error.h"
#include "tokenizer.h"
#include "parser.h"
#include "global.h"
#include "emitter.h"

Token lookahead;
int currentAddress = 0;

#define TEMP_BASEADDR 5

int parse(FILE *file)
{
  // we setbuf because we want printf to
  // output before stuff like segfaults happen
  setbuf(stdout, NULL);

  char data[BUFFER_SIZE + 3];
  Buffer buffer;

  initBuffer(&buffer, data, file);
  lookahead = nextToken(&buffer);

  IR ir = {0};
  ir.nodes = malloc(sizeof(IRNode) * IR_SIZE);
  ir.current = 0;

  while (lookahead != TK_EOF && statement(&buffer, &ir))
    ;

  emit(&ir);

  return 0;
}

bool match(Buffer *buffer, Token t)
{
  if (t == lookahead)
  {
    lookahead = nextToken(buffer);
    return true;
  }
  else
  {
    error("syntax error");
  }

  return false;
}

bool statement(Buffer *buffer, IR *ir)
{
  switch (lookahead)
  {
  case TK_ADD:
  {
    emitAdd();
    return match(buffer, TK_ADD);
  }
  case TK_SUB:
  {
    emitSub();
    return match(buffer, TK_SUB);
  }
  case TK_AND:
  {
    emitLogicalAnd();
    return match(buffer, TK_AND);
  }
  case TK_EQ:
  {
    emitEq();
    return match(buffer, TK_EQ);
  }
  case TK_GT:
  {
    emitGt();
    return match(buffer, TK_GT);
  }
  case TK_LT:
  {
    emitLt();
    return match(buffer, TK_LT);
  }
  case TK_NEG:
  {
    emitNeg();
    return match(buffer, TK_NEG);
  }
  case TK_NOT:
  {
    emitLogicalNot();
    return match(buffer, TK_NOT);
  }
  case TK_OR:
  {
    emitLogicalOr();
    return match(buffer, TK_OR);
  }
  case TK_POP:
  {
    match(buffer, TK_POP);
    return pop(buffer);
  }
  case TK_PUSH:
  {
    match(buffer, TK_PUSH);
    return push(buffer);
  }
  case TK_LABEL: {
    match(buffer, TK_LABEL);

    emitLabel(symtable[tokenval].lexptr);

    return match(buffer, TK_IDENTIFIER);
  }
  case TK_FUNCTION: {
    match(buffer, TK_FUNCTION);

    // must emit a label that is the "function address"
    emitLabel(symtable[tokenval].lexptr);

    match(buffer, TK_IDENTIFIER);

    // initialize the local variables, first tokenval entries in LCL
    for (int i = 0; i < tokenval; i++) {
      emitInitLocal(i);
    }

    return match(buffer, TK_NUMBER);
  }
  case TK_RETURN: {
    match(buffer, TK_RETURN);

    emitReturn();

    return true;
  }
  case TK_CALL: {
    match(buffer, TK_CALL);

    char *id = symtable[tokenval].lexptr;

    match(buffer, TK_IDENTIFIER);

    // generate and push return address Xxx$ret.1
    // push caller's stack frame
    // LCL, ARG, THIS, THAT
    // ARG = SPI - 5 - nArgs (tokenval)
    // LCL = SP
    // emit goto functionName
    // emit return label

    return match(buffer, TK_NUMBER);
  }
  case TK_IF_GOTO: {
    match(buffer, TK_IF_GOTO);

    emitIfGoto(symtable[tokenval].lexptr);

    return match(buffer, TK_IDENTIFIER);
  }
  case TK_GOTO: {
    match(buffer, TK_GOTO);

    emitGoto(symtable[tokenval].lexptr);

    return match(buffer, TK_IDENTIFIER);
  }
  default:
    error("encountered unexpected token");
  }

  return true;
};

bool pop(Buffer *buffer)
{
  switch (lookahead)
  {
  case TK_STATIC:
  {
    match(buffer, TK_STATIC);

    emitPopStatic(tokenval);

    return match(buffer, TK_NUMBER);
  }
  case TK_POINTER:
  {
    match(buffer, TK_POINTER);

    emitPopPointer(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_LOCAL:
  {
    match(buffer, TK_LOCAL);

    emitPopLocal(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_ARGUMENT:
  {
    match(buffer, TK_ARGUMENT);

    emitPopArgument(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_THIS:
  {
    match(buffer, TK_THIS);

    emitPopThis(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_THAT:
  {
    match(buffer, TK_THAT);
    // pop stack into that n

    emitPopThat(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_TEMP:
  {
    match(buffer, TK_TEMP);

    emitPopTemp(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  default:
    fprintf(stderr, "encountered unimplemented pop: %d\n", lookahead);
    error("parse error");
    return false;
  }
}

bool push(Buffer *buffer)
{
  switch (lookahead)
  {
  case TK_STATIC:
  {
    match(buffer, TK_STATIC);

    // this needs to use the filename dynamically
    printf("@sample.%d\n", tokenval);
    printf("D=M\n");

    emitPushD();

    return match(buffer, TK_NUMBER);
  }
  case TK_POINTER:
  {
    match(buffer, TK_POINTER);

    if (tokenval == 0)
    {
      // we want to take the actual value of @THIS
      // and push it to the stack
      printf("@THIS\n");
      printf("D=M\n");
      emitPushD();
    }
    else if (tokenval == 1)
    {
      printf("@THAT\n");
      printf("D=M\n");
      emitPushD();
    }
    else
    {
      return error("tried to pop pointer with a value other than 0/1\n");
    }

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_CONSTANT:
  {
    match(buffer, TK_CONSTANT);

    // *SP = i
    printf("@%d\n", tokenval);
    printf("D=A\n");

    emitPushD();

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_LOCAL:
  {
    match(buffer, TK_LOCAL);

    // put the nth local on the stack

    // addr = local + n
    printf("@%d\n", tokenval);
    printf("D=A\n");
    printf("@LCL\n");
    printf("D=D+M\n");

    // dereference local + n into D
    printf("A=D\n");
    printf("D=M\n");

    emitPushD();

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_ARGUMENT:
  {
    match(buffer, TK_ARGUMENT);

    // put the nth argument on the stack

    // addr = argument + n
    printf("@%d\n", tokenval);
    printf("D=A\n");
    printf("@ARG\n");
    printf("D=D+M\n");

    // dereference arg + n into D
    printf("A=D\n");
    printf("D=M\n");

    emitPushD();

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_THIS:
  {
    match(buffer, TK_THIS);

    // put the nth this on the stack

    // addr = this + n
    printf("@%d\n", tokenval);
    printf("D=A\n");
    printf("@THIS\n");
    printf("D=D+M\n");

    // dereference this + n into D
    printf("A=D\n");
    printf("D=M\n");

    emitPushD();

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_THAT:
  {
    match(buffer, TK_THAT);
    // put the nth that on the stack

    // addr = that + n
    printf("@%d\n", tokenval);
    printf("D=A\n");
    printf("@THAT\n");
    printf("D=D+M\n");

    // dereference local + n into D
    printf("A=D\n");
    printf("D=M\n");

    emitPushD();

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_TEMP:
  {
    match(buffer, TK_TEMP);
    // push the nth temp to the stack

    // addr = that + n
    int addr = TEMP_BASEADDR + tokenval;

    // *SP = addr*
    printf("@%d\n", addr);
    printf("D=M\n"); // D gets addr*
    emitPushD();
    match(buffer, TK_NUMBER);
    return true;
  }
  default:
    fprintf(stderr, "encountered unimplemented push: %d\n", lookahead);
    error("parse error");
    return false;
  }
}