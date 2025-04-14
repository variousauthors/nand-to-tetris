#include "global.h"
#include "symbol.h"
#include "tokenizer.h"
#include "init.h"

struct entry keywords[] = {
    "add", TK_ADD, 0,
    "sub", TK_SUB, 0,
    "neg", TK_NEG, 0,
    "eq", TK_EQ, 0,
    "gt", TK_GT, 0,
    "lt", TK_LT, 0,
    "and", TK_AND, 0,
    "or", TK_OR, 0,
    "not", TK_NOT, 0,
    "pop", TK_POP, 0,
    "push", TK_PUSH, 00,
    "static", TK_STATIC, 0,
    "local", TK_LOCAL, 0,
    "constant", TK_CONSTANT, 0,
    "this", TK_THIS, 0,
    "that", TK_THAT, 0,
    "argument", TK_ARGUMENT, 0,
    "temp", TK_TEMP, 0,
    "pointer", TK_POINTER, 0,
    "label", TK_LABEL, 0,
    "if-goto", TK_IF_GOTO, 0,
    "goto", TK_GOTO, 0,
    "function", TK_FUNCTION, 0,
    0, 0};

void init()
{
  struct entry *p;
  for (p = keywords; p->token; p++)
  {
    insert(p->lexptr, p->token, p->value);
  }

  // we will scan the entire program and build up
  // a complete symbol table
}