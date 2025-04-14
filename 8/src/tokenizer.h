#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "buffer.h"

typedef enum
{
  TK_NONE,
  TK_ADD,
  TK_AND,
  TK_ARGUMENT,
  TK_CONSTANT,
  TK_EQ,
  TK_GT,
  TK_LOCAL,
  TK_LT,
  TK_NEG,
  TK_NOT,
  TK_OR,
  TK_POINTER,
  TK_LABEL,
  TK_IF_GOTO,
  TK_GOTO,
  TK_IDENTIFIER,
  TK_POP,
  TK_PUSH,
  TK_STATIC,
  TK_SUB,
  TK_TEMP,
  TK_THAT,
  TK_THIS,
  TK_EOF,
  TK_NUMBER,
} Token;

extern int tokenval;

Token nextToken(Buffer *buffer);

#endif