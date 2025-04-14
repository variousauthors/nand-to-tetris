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
  lineno = 1;

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

    emitCall(id, currentFile, tokenval);

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

    emitPushStatic(tokenval);

    return match(buffer, TK_NUMBER);
  }
  case TK_POINTER:
  {
    match(buffer, TK_POINTER);

    emitPushPointer(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_CONSTANT:
  {
    match(buffer, TK_CONSTANT);

    emitPushConstant(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_LOCAL:
  {
    match(buffer, TK_LOCAL);

    emitPushLocal(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_ARGUMENT:
  {
    match(buffer, TK_ARGUMENT);

    emitPushArgument(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_THIS:
  {
    match(buffer, TK_THIS);

    emitPushThis(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_THAT:
  {
    match(buffer, TK_THAT);

    emitPushThat(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_TEMP:
  {
    match(buffer, TK_TEMP);

    emitPushTemp(tokenval);

    match(buffer, TK_NUMBER);
    return true;
  }
  default:
    fprintf(stderr, "encountered unimplemented push: %d\n", lookahead);
    error("parse error");
    return false;
  }
}