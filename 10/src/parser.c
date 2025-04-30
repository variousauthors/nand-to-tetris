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

bool match(Buffer *buffer, Token t);
bool aToken(Buffer *buffer);

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

  emitXMLOpenTag("tokens");

  while (lookahead != TK_EOF && aToken(&buffer))
    ;

  emitXMLCloseTag("tokens");
  // emit(&ir);

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

bool aToken(Buffer *buffer)
{
  switch (lookahead)
  {
  case TK_CLASS:
  case TK_CONSTRUCTOR:
  case TK_FUNCTION:
  case TK_METHOD:
  case TK_FIELD:
  case TK_STATIC:
  case TK_VAR:
  case TK_INT:
  case TK_CHAR:
  case TK_BOOLEAN:
  case TK_VOID:
  case TK_TRUE:
  case TK_FALSE:
  case TK_NULL:
  case TK_THIS:
  case TK_LET:
  case TK_DO:
  case TK_IF:
  case TK_ELSE:
  case TK_WHILE:
  case TK_RETURN:
  {
    emitXMLPrimitive("keyword", symtable[tokenval].lexptr);
    return match(buffer, lookahead);
  }
  case TK_IDENTIFIER:
  {
    emitXMLPrimitive("identifier", symtable[tokenval].lexptr);
    return match(buffer, lookahead);
  }
  case TK_BRACE_L:
  {
    emitXMLPrimitive("symbol", "{");
    return match(buffer, lookahead);
  }
  case TK_BRACE_R:
  {
    emitXMLPrimitive("symbol", "}");
    return match(buffer, lookahead);
  }
  case TK_PAREN_L:
  {
    emitXMLPrimitive("symbol", "(");
    return match(buffer, lookahead);
  }
  case TK_PAREN_R:
  {
    emitXMLPrimitive("symbol", ")");
    return match(buffer, lookahead);
  }
  case TK_BRACKET_L:
  {
    emitXMLPrimitive("symbol", "[");
    return match(buffer, lookahead);
  }
  case TK_BRACKET_R:
  {
    emitXMLPrimitive("symbol", "]");
    return match(buffer, lookahead);
  }
  case TK_DOT:
  {
    emitXMLPrimitive("symbol", ".");
    return match(buffer, lookahead);
  }
  case TK_COMMA:
  {
    emitXMLPrimitive("symbol", ",");
    return match(buffer, lookahead);
  }
  case TK_SEMI:
  {
    emitXMLPrimitive("symbol", ";");
    return match(buffer, lookahead);
  }
  case TK_PLUS:
  {
    emitXMLPrimitive("symbol", "+");
    return match(buffer, lookahead);
  }
  case TK_MINUS:
  {
    emitXMLPrimitive("symbol", "-");
    return match(buffer, lookahead);
  }
  case TK_ASTERISK:
  {
    emitXMLPrimitive("symbol", "*");
    return match(buffer, lookahead);
  }
  case TK_SLASH:
  {
    emitXMLPrimitive("symbol", "/");
    return match(buffer, lookahead);
  }
  case TK_AMP:
  {
    emitXMLPrimitive("symbol", "&amp;");
    return match(buffer, lookahead);
  }
  case TK_BAR:
  {
    emitXMLPrimitive("symbol", "|");
    return match(buffer, lookahead);
  }
  case TK_ANGLE_BRACKET_L:
  {
    emitXMLPrimitive("symbol", "&lt;");
    return match(buffer, lookahead);
  }
  case TK_ANGLE_BRACKET_R:
  {
    emitXMLPrimitive("symbol", "&gt;");
    return match(buffer, lookahead);
  }
  case TK_EQUAL:
  {
    emitXMLPrimitive("symbol", "=");
    return match(buffer, lookahead);
  }
  case TK_TILDE:
  {
    emitXMLPrimitive("symbol", "~");
    return match(buffer, lookahead);
  }
  case TK_STRING: {
    emitXMLPrimitive("stringConstant", identifierBuffer);
    return match(buffer, lookahead);
  }
  case TK_INTEGER: {
    emitXMLPrimitiveInteger("integerConstant", tokenval);
    return match(buffer, lookahead);
  }
  default:
    fprintf(stderr, "token: %d\n", lookahead);
    error("encountered unrecognized token");
    break;
  }
  return false;
}
