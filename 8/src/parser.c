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

  fclose(file);
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

/** register D will get the value baseaddr + offset for the given segment */
void emitAGetsBasePlusOffset(char *segment, int offset)
{
  printf("@%d\n", offset); // eg @7 if n = 7
  printf("D=A\n");
  printf("@%s\n", segment); // eg @LCL if seg = LCL
  printf("A=D+M\n");
}

/** register D will get the value baseaddr + offset for the given segment */
void emitDGetsBasePlusOffset(char *segment, int offset)
{
  printf("@%d\n", offset); // eg @7 if n = 7
  printf("D=A\n");
  printf("@%s\n", segment); // eg @LCL if seg = LCL
  printf("D=D+M\n");
}

void emitGetDFromStack()
{
  printf("@SP\n");
  printf("A=M\n");
  printf("D=M\n");
}

void emitWriteDToStack()
{
  printf("@SP\n");
  printf("A=M\n");
  printf("M=D\n");
}

void emitDecrementSP()
{
  printf("@SP\n");
  printf("M=M-1\n");
}

void emitIncrementSP()
{
  printf("@SP\n");
  printf("M=M+1\n");
}

void emitPushD()
{
  emitWriteDToStack();
  emitIncrementSP();
}

void emitPopIntoD()
{
  emitDecrementSP();
  emitGetDFromStack();
}

bool statement(Buffer *buffer, IR *ir)
{
  switch (lookahead)
  {
  case TK_ADD:
  {
    match(buffer, TK_ADD);
    return add(buffer);
  }
  case TK_SUB:
  {
    match(buffer, TK_SUB);
    return sub(buffer);
  }
  case TK_AND:
  {
    match(buffer, TK_AND);
    return logicalAnd(buffer);
  }
  case TK_EQ:
  {
    match(buffer, TK_EQ);
    return eq(buffer);
  }
  case TK_GT:
  {
    match(buffer, TK_GT);
    return gt(buffer);
  }
  case TK_LT:
  {
    match(buffer, TK_LT);
    return lt(buffer);
  }
  case TK_NEG:
  {
    match(buffer, TK_NEG);
    return neg(buffer);
  }
  case TK_NOT:
  {
    match(buffer, TK_NOT);
    return logicalNot(buffer);
  }
  case TK_OR:
  {
    match(buffer, TK_OR);
    return logicalOr(buffer);
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

    // emit a label into the asm
    printf("(%s)\n", symtable[tokenval].lexptr);

    return match(buffer, TK_IDENTIFIER);
  }
  case TK_FUNCTION: {
    match(buffer, TK_FUNCTION);

    // must emit a label that is the "function address"
    printf("(%s)\n", symtable[tokenval].lexptr);

    match(buffer, TK_IDENTIFIER);

    // initialize the local variables, first tokenval entries in LCL
    for (int i = 0; i < tokenval; i++) {
      emitAGetsBasePlusOffset("LCL", i);
      printf("M=0\n");
    }

    return match(buffer, TK_NUMBER);
  }
  case TK_RETURN: {
    match(buffer, TK_RETURN);

    // the return address is guaranteed to be on the stack
    // endFrame = LCL
    // returnAdd = *(LCL - 5)

    // value at the top of the stack is the return value
    // put that into ARG 0
    // *ARG = pop
    // SP = ARG + 1

    // restore context
    // THAT = *(endframe - 1)
    // THIS = *(endframe - 2)
    // ARG = *(endframe - 3)
    // LCL = *(endframe - 4)

    // finally GOTO retAddr


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

    // consume a value from the stack
    // do a jump if that is gt zero
    emitPopIntoD();
    printf("@%s\n", symtable[tokenval].lexptr);
    printf("D;JGT\n");

    return match(buffer, TK_IDENTIFIER);
  }
  case TK_GOTO: {
    match(buffer, TK_GOTO);

    // just jump

    printf("@%s\n", symtable[tokenval].lexptr);
    printf("0;JEQ\n");

    return match(buffer, TK_IDENTIFIER);
  }
  default:
    error("encountered unexpected token");
  }

  return true;
};

bool add(Buffer *buffer)
{
  // get a value from the stack
  // get another value from the stack
  // add them, put that back on the stack

  // pop a value
  emitPopIntoD();

  // store it in R13
  printf("@R13\n");
  printf("M=D\n");

  // pop a value into D
  emitPopIntoD();

  // retrieve R13 and add
  printf("@R13\n");
  printf("D=D+M\n");

  emitPushD();

  return true;
}
bool sub(Buffer *buffer)
{
  // pop a value
  emitPopIntoD();

  // store it in R13
  printf("@R13\n");
  printf("M=D\n");

  emitPopIntoD();

  // retrieve R13 and add
  printf("@R13\n");
  printf("D=D-M\n");

  emitPushD();

  return true;
}
bool neg(Buffer *buffer)
{
  error("neg\n");
  return true;
}
bool eq(Buffer *buffer)
{
  error("eq\n");
  return true;
}
bool gt(Buffer *buffer)
{
  error("gt\n");
  return true;
}
bool lt(Buffer *buffer)
{
  error("lt\n");
  return true;
}
bool logicalAnd(Buffer *buffer)
{
  error("logicalAnd\n");
  return true;
}
bool logicalOr(Buffer *buffer)
{
  error("logicalOr\n");
  return true;
}
bool logicalNot(Buffer *buffer)
{
  error("logicalNot\n");
  return true;
}

bool pop(Buffer *buffer)
{
  switch (lookahead)
  {
  case TK_STATIC:
  {
    match(buffer, TK_STATIC);

    // this needs to use the filename dynamically
    emitPopIntoD();

    printf("@sample.%d\n", tokenval);
    printf("M=D\n");

    return match(buffer, TK_NUMBER);
  }
  case TK_POINTER:
  {
    match(buffer, TK_POINTER);

    if (tokenval == 0)
    {
      // we want to set THIS to the value we pop, not dereference it
      emitPopIntoD();
      printf("@THIS\n");
      printf("M=D\n");
    }
    else if (tokenval == 1)
    {
      emitPopIntoD();
      printf("@THAT\n");
      printf("M=D\n");
    }
    else
    {
      return error("tried to pop pointer with a value other than 0/1\n");
    }

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_LOCAL:
  {
    match(buffer, TK_LOCAL);

    // addr = local + n
    emitDGetsBasePlusOffset("LCL", tokenval);

    printf("@R15\n"); // R13, R14, and R15 are our scratch registers
    printf("M=D\n");

    emitPopIntoD();

    printf("@R15\n");
    printf("A=M\n"); // jump to the addr
    printf("M=D\n"); // store at addr

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_ARGUMENT:
  {
    match(buffer, TK_ARGUMENT);

    // pop the stack into argument n

    // addr = local + n
    emitDGetsBasePlusOffset("ARG", tokenval);

    printf("@R15\n"); // R13, R14, and R15 are our scratch registers
    printf("M=D\n");

    emitPopIntoD();

    printf("@R15\n");
    printf("A=M\n"); // jump to the addr
    printf("M=D\n"); // store at addr

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_THIS:
  {
    match(buffer, TK_THIS);

    // pop this n

    // addr = this + n
    emitDGetsBasePlusOffset("THIS", tokenval);

    printf("@R15\n"); // R13, R14, and R15 are our scratch registers
    printf("M=D\n");

    emitPopIntoD();

    printf("@R15\n");
    printf("A=M\n"); // jump to the addr
    printf("M=D\n"); // store at addr

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_THAT:
  {
    match(buffer, TK_THAT);
    // pop stack into that n

    // addr = that + n
    emitDGetsBasePlusOffset("THAT", tokenval);

    printf("@R15\n"); // R13, R14, and R15 are our scratch registers
    printf("M=D\n");

    emitPopIntoD();

    printf("@R15\n");
    printf("A=M\n"); // jump to the addr
    printf("M=D\n"); // store at addr

    match(buffer, TK_NUMBER);
    return true;
  }
  case TK_TEMP:
  {
    match(buffer, TK_TEMP);

    // addr = temp + n
    int addr = TEMP_BASEADDR + tokenval;

    emitPopIntoD();

    printf("@%d\n", addr); // constant address
    printf("M=D\n");       // store at addr

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