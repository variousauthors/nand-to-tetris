#include "global.h"
#include "symbol.h"
#include "tokenizer.h"
#include "init.h"

struct entry keywords[] = {
    "class", TK_CLASS, 0,
    "constructor", TK_CONSTRUCTOR, 0,
    "function", TK_FUNCTION, 0,
    "method", TK_METHOD, 0,
    "field", TK_FIELD, 0,
    "static", TK_STATIC, 0,
    "var", TK_VAR, 0,
    "int", TK_INT, 0,
    "char", TK_CHAR, 0,
    "boolean", TK_BOOLEAN, 0,
    "void", TK_VOID, 0,
    "true", TK_TRUE, 0,
    "false", TK_FALSE, 0,
    "null", TK_NULL, 0,
    "this", TK_THIS, 0,
    "let", TK_LET, 0,
    "do", TK_DO, 0,
    "if", TK_IF, 0,
    "else", TK_ELSE, 0,
    "while", TK_WHILE, 0,
    "return", TK_RETURN, 0,
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