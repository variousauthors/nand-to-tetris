#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "buffer.h"

typedef enum
{
  TK_NONE,

  // keywords,
  TK_CLASS,
  TK_CONSTRUCTOR,
  TK_FUNCTION,
  TK_METHOD,
  TK_FIELD,
  TK_STATIC,
  TK_VAR,
  TK_INT,
  TK_CHAR,
  TK_BOOLEAN,
  TK_VOID,
  TK_TRUE,
  TK_FALSE,
  TK_NULL,
  TK_THIS,
  TK_LET,
  TK_DO,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_RETURN,

  // symbols
  TK_BRACE_L,
  TK_BRACE_R,
  TK_PAREN_L,
  TK_PAREN_R,
  TK_BRACKET_L,
  TK_BRACKET_R,
  TK_DOT,
  TK_COMMA,
  TK_SEMI,
  TK_PLUS,
  TK_MINUS,
  TK_ASTERISK,
  TK_SLASH,
  TK_AMP,
  TK_BAR,
  TK_ANGLE_BRACKET_L,
  TK_ANGLE_BRACKET_R,
  TK_EQUAL,
  TK_TILDE,

  // composites
  TK_INTEGER,
  TK_STRING,
  TK_IDENTIFIER,

  TK_EOF,
} Token;

extern int tokenval;

Token nextToken(Buffer *buffer);

#endif