#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>

#include "buffer.h"
#include "error.h"
#include "tokenizer.h"
#include "parser.h"
#include "emitter.h"

Token lookahead;
int currentAddress = 0;

// two global symbol tables
ScopedSymbolTable classSymbolTable;
ScopedSymbolTable subroutineSymbolTable;

#define TEMP_BASEADDR 5

bool statements(Buffer *buffer);
bool match(Buffer *buffer, Token t);
bool class(Buffer *buffer);
bool expression(Buffer *buffer);
bool functionCall(Buffer *buffer, char *subroutineName);
bool methodCall(Buffer *buffer, char *subjectName);

int parse(FILE *file)
{
  // we setbuf because we want printf to
  // output before stuff like segfaults happen
  setbuf(stdout, NULL);

  char data[BUFFER_SIZE + 3];
  Buffer buffer;

  initBuffer(&buffer, data, file);
  lookahead = nextToken(&buffer);

  lineno = 1;

  while (lookahead != TK_EOF && class(&buffer))
    ;

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
    fprintf(stderr, "expected %d, encountered %d\n", t, lookahead);
    error("syntax error");
  }

  return false;
}

bool identifierSubroutineDeclaration(Buffer *buffer)
{
  emitIdentifierSubroutine(identifierBuffer, "declaration");
  match(buffer, TK_IDENTIFIER);
  return true;
}

bool varNameIdentifier(Buffer *buffer, ScopedSymbolTable *table, ScopedSymbolTableEntry *entry)
{
  entry->name = &symtable[tokenval];
  defineScopedSymbol(table, entry->name, entry->type, entry->kind);

  match(buffer, TK_IDENTIFIER);
  emitIdentifierSpecial(identifierBuffer, "declaration");
  return true;
}

bool additionalVarName(Buffer *buffer, ScopedSymbolTable *scopedSymbolTable, ScopedSymbolTableEntry *entry)
{
  if (lookahead != TK_COMMA)
  {
    return false;
  }

  match(buffer, TK_COMMA);
  emitSymbol(",");

  varNameIdentifier(buffer, scopedSymbolTable, entry);

  return true;
}

bool isType(Token lookahead)
{
  return lookahead == TK_INT || lookahead == TK_CHAR || lookahead == TK_BOOLEAN || lookahead == TK_IDENTIFIER;
}

bool varType(Buffer *buffer, ScopedSymbolTableEntry *entry)
{
  switch (lookahead)
  {
  case TK_INT:
  {
    entry->type = &symtable[tokenval];
    match(buffer, TK_INT);
    emitXMLPrimitive("keyword", "int");
    return true;
  }
  case TK_CHAR:
  {
    entry->type = &symtable[tokenval];
    match(buffer, TK_CHAR);
    emitXMLPrimitive("keyword", "char");
    return true;
  }
  case TK_BOOLEAN:
  {
    entry->type = &symtable[tokenval];
    match(buffer, TK_BOOLEAN);
    emitXMLPrimitive("keyword", "boolean");
    return true;
  }
  case TK_IDENTIFIER:
  {
    // emit first because the buffer will be overwritten
    // if the next token is also an identifier
    entry->type = &symtable[tokenval];
    emitIdentifierClass(identifierBuffer, "reference");
    match(buffer, TK_IDENTIFIER);
    return true;
  }
  default:
    fprintf(stderr, "expected int | char | boolean | classname but got %d\n", lookahead);
    error("error while parsing type");
    return false;
  }
}

/** these are not part of symbol table stuff */
bool returnType(Buffer *buffer)
{
  switch (lookahead)
  {
  case TK_INT:
  {
    match(buffer, TK_INT);
    emitXMLPrimitive("keyword", "int");
    return true;
  }
  case TK_CHAR:
  {
    match(buffer, TK_CHAR);
    emitXMLPrimitive("keyword", "char");
    return true;
  }
  case TK_BOOLEAN:
  {
    match(buffer, TK_BOOLEAN);
    emitXMLPrimitive("keyword", "boolean");
    return true;
  }
  case TK_IDENTIFIER:
  {
    // emit first because the buffer will be overwritten
    // if the next token is also an identifier
    emitIdentifierClass(identifierBuffer, "reference");
    match(buffer, TK_IDENTIFIER);
    return true;
  }
  default:
    fprintf(stderr, "expected int | char | boolean | classname but got %d\n", lookahead);
    error("error while parsing type");
    return false;
  }
}

bool voidType(Buffer *buffer)
{
  if (lookahead == TK_VOID)
  {
    match(buffer, TK_VOID);
    emitKeyword("void");
    return true;
  }

  return returnType(buffer);
}

bool parameter(Buffer *buffer, ScopedSymbolTable *table)
{
  ScopedSymbolTableEntry entry;
  entry.kind = VK_ARG;

  varType(buffer, &entry);
  varNameIdentifier(buffer, table, &entry);

  return true;
}

bool additionalParameter(Buffer *buffer, ScopedSymbolTable *table)
{
  if (lookahead != TK_COMMA)
  {
    return false;
  }

  match(buffer, TK_COMMA);
  emitSymbol(",");
  return parameter(buffer, table);
}

bool parameterList(Buffer *buffer, ScopedSymbolTable *scopedSymbolTable)
{
  emitXMLOpenTag("parameterList");

  if (isType(lookahead))
  {
    // we have a parameter list
    parameter(buffer, scopedSymbolTable);

    while (additionalParameter(buffer, scopedSymbolTable))
      ;
  }

  emitXMLCloseTag("parameterList");
  return true;
}

bool varDecDetails(Buffer *buffer, ScopedSymbolTable *table, ScopedSymbolTableEntry *entry)
{
  // type varName (, varName)* ;
  varType(buffer, entry);
  varNameIdentifier(buffer, table, entry);

  while (additionalVarName(buffer, table, entry))
    ;

  match(buffer, TK_SEMI);
  emitXMLPrimitive("symbol", ";");

  return true;
}

bool varDec(Buffer *buffer, ScopedSymbolTable *table)
{
  if (lookahead != TK_VAR)
  {
    return false;
  }

  ScopedSymbolTableEntry entry;

  match(buffer, TK_VAR);
  emitXMLOpenTag("varDec");
  emitKeyword("var");

  entry.kind = VK_VAR;

  varDecDetails(buffer, table, &entry);

  emitXMLCloseTag("varDec");
  return true;
}

bool unaryOp(Buffer *buffer)
{
  switch (lookahead)
  {
  case TK_TILDE:
  {
    match(buffer, TK_TILDE);
    emitSymbol("~");
    break;
  }
  case TK_MINUS:
  {
    match(buffer, TK_MINUS);
    emitSymbol("-");
    break;
  }
  default:
    fprintf(stderr, "got %d\n", lookahead);
    error("while parsing unary op");
    return false;
  }

  return true;
}

bool term(Buffer *buffer)
{
  emitXMLOpenTag("term");

  switch (lookahead)
  {
  case TK_TILDE:
  case TK_MINUS:
  {
    unaryOp(buffer);
    term(buffer);
    break;
  }
  case TK_INTEGER:
  {
    // emit first because tokenval gets clobbered
    emitXMLPrimitiveInteger("integerConstant", tokenval);
    match(buffer, TK_INTEGER);
    break;
  }
  case TK_STRING:
  {
    // we emit first because string buffer might get
    // overwritten if we match first
    emitXMLPrimitive("stringConstant", identifierBuffer);
    match(buffer, TK_STRING);
    break;
  }
  case TK_PAREN_L:
  {
    // ( expression )
    match(buffer, TK_PAREN_L);
    emitSymbol("(");
    expression(buffer);
    match(buffer, TK_PAREN_R);
    emitSymbol(")");
    break;
  }
  case TK_CLASS:
  {
    match(buffer, TK_CLASS);
    emitKeyword("class");
    break;
  }
  case TK_CONSTRUCTOR:
  {
    match(buffer, TK_CONSTRUCTOR);
    emitKeyword("constructor");
    break;
  }
  case TK_FUNCTION:
  {
    match(buffer, TK_FUNCTION);
    emitKeyword("function");
    break;
  }
  case TK_METHOD:
  {
    match(buffer, TK_METHOD);
    emitKeyword("method");
    break;
  }
  case TK_FIELD:
  {
    match(buffer, TK_FIELD);
    emitKeyword("field");
    break;
  }
  case TK_STATIC:
  {
    match(buffer, TK_STATIC);
    emitKeyword("static");
    break;
  }
  case TK_VAR:
  {
    match(buffer, TK_VAR);
    emitKeyword("var");
    break;
  }
  case TK_INT:
  {
    match(buffer, TK_INT);
    emitKeyword("int");
    break;
  }
  case TK_CHAR:
  {
    match(buffer, TK_CHAR);
    emitKeyword("char");
    break;
  }
  case TK_BOOLEAN:
  {
    match(buffer, TK_BOOLEAN);
    emitKeyword("boolean");
    break;
  }
  case TK_VOID:
  {
    match(buffer, TK_VOID);
    emitKeyword("void");
    break;
  }
  case TK_TRUE:
  {
    match(buffer, TK_TRUE);
    emitKeyword("true");
    break;
  }
  case TK_FALSE:
  {
    match(buffer, TK_FALSE);
    emitKeyword("false");
    break;
  }
  case TK_NULL:
  {
    match(buffer, TK_NULL);
    emitKeyword("null");
    break;
  }
  case TK_LET:
  {
    match(buffer, TK_LET);
    emitKeyword("let");
    break;
  }
  case TK_DO:
  {
    match(buffer, TK_DO);
    emitKeyword("do");
    break;
  }
  case TK_IF:
  {
    match(buffer, TK_IF);
    emitKeyword("if");
    break;
  }
  case TK_ELSE:
  {
    match(buffer, TK_ELSE);
    emitKeyword("else");
    break;
  }
  case TK_WHILE:
  {
    match(buffer, TK_WHILE);
    emitKeyword("while");
    break;
  }
  case TK_RETURN:
  {
    match(buffer, TK_RETURN);
    emitKeyword("return");
    break;
  }
  case TK_THIS:
  {
    match(buffer, TK_THIS);
    emitKeyword("this");
    break;
  }
  case TK_IDENTIFIER:
  {
    // varName || varName[expression] || subroutineCall
    char *id = symtable[tokenval].lexptr;

    match(buffer, TK_IDENTIFIER);

    if (lookahead == TK_BRACKET_L)
    {
      // varName[expression]
      emitIdentifierSpecial(id, "reference");

      match(buffer, TK_BRACKET_L);
      emitSymbol("[");
      expression(buffer);
      match(buffer, TK_BRACKET_R);
      emitSymbol("]");
    }
    else if (lookahead == TK_DOT)
    {
      methodCall(buffer, id);
    }
    else if (lookahead == TK_PAREN_L)
    {
      functionCall(buffer, id);
    }
    else
    {
      // varName
      emitIdentifierSpecial(id, "reference");
    }

    break;
  }
  default:
    error("failed to parse expression");
  }

  emitXMLCloseTag("term");

  return true;
}

bool op(Buffer *buffer)
{
  switch (lookahead)
  {
  case TK_PLUS:
  {
    match(buffer, TK_PLUS);
    emitSymbol("+");
    break;
  }
  case TK_MINUS:
  {
    match(buffer, TK_MINUS);
    emitSymbol("-");
    break;
  }
  case TK_ASTERISK:
  {
    match(buffer, TK_ASTERISK);
    emitSymbol("*");
    break;
  }
  case TK_SLASH:
  {
    match(buffer, TK_SLASH);
    emitSymbol("/");
    break;
  }
  case TK_AMP:
  {
    match(buffer, TK_AMP);
    emitSymbol("&");
    break;
  }
  case TK_BAR:
  {
    match(buffer, TK_BAR);
    emitSymbol("|");
    break;
  }
  case TK_ANGLE_BRACKET_L:
  {
    match(buffer, TK_ANGLE_BRACKET_L);
    emitSymbol("<");
    break;
  }
  case TK_ANGLE_BRACKET_R:
  {
    match(buffer, TK_ANGLE_BRACKET_R);
    emitSymbol(">");
    break;
  }
  case TK_EQUAL:
  {
    match(buffer, TK_EQUAL);
    emitSymbol("=");
    break;
  }
  default:
    fprintf(stderr, "expected binary operator got %d\n", lookahead);
    error("error while parsing op");
    return false;
  }

  return true;
}

bool isOp(Token token)
{
  return token == TK_PLUS || token == TK_MINUS || token == TK_ASTERISK || token == TK_SLASH || token == TK_AMP || token == TK_BAR || token == TK_ANGLE_BRACKET_L || token == TK_ANGLE_BRACKET_R || token == TK_EQUAL;
}

bool opTerm(Buffer *buffer)
{
  if (!isOp(lookahead))
  {
    return false;
  }

  op(buffer);
  return term(buffer);
}

bool expression(Buffer *buffer)
{
  emitXMLOpenTag("expression");

  // term (op term)*

  term(buffer);
  while (opTerm(buffer))
    ;

  emitXMLCloseTag("expression");

  return true;
}

bool additionalExpressions(Buffer *buffer)
{
  if (lookahead != TK_COMMA)
  {
    return false;
  }

  match(buffer, TK_COMMA);
  emitSymbol(",");

  return expression(buffer);
}

bool expressionList(Buffer *buffer, bool isEmpty)
{
  emitXMLOpenTag("expressionList");

  // expression and term have no way to express "emptiness"
  // so the caller will need to have the context to know
  // whether this list terminates immediately or not
  // (based on the lookahead)
  if (!isEmpty)
  {
    expression(buffer);
    while (additionalExpressions(buffer))
      ;
  }

  emitXMLCloseTag("expressionList");

  return true;
}

bool functionCall(Buffer *buffer, char *subroutineName)
{
  // subroutineName ( expressionList )
  emitIdentifierSubroutine(subroutineName, "reference");
  match(buffer, TK_PAREN_L);
  emitSymbol("(");

  expressionList(buffer, lookahead == TK_PAREN_R);

  match(buffer, TK_PAREN_R);
  emitSymbol(")");

  return true;
}

bool methodCall(Buffer *buffer, char *subjectName)
{
  // ( className | varName ) . subroutineName ( expressionList )
  match(buffer, TK_DOT);
  // (className | varName) . subroutineName ( expressionList )
  char *id2 = symtable[tokenval].lexptr;

  emitIdentifierClass(subjectName, "reference");
  emitSymbol(".");
  emitIdentifierSubroutine(id2, "reference");
  match(buffer, TK_IDENTIFIER);
  match(buffer, TK_PAREN_L);
  emitSymbol("(");

  expressionList(buffer, lookahead == TK_PAREN_R);

  match(buffer, TK_PAREN_R);
  emitSymbol(")");

  return true;
}

bool subroutineCall(Buffer *buffer)
{
  if (lookahead != TK_IDENTIFIER)
  {
    return false;
  }

  char *id = symtable[tokenval].lexptr;
  match(buffer, TK_IDENTIFIER);

  if (lookahead == TK_DOT)
  {
    return methodCall(buffer, id);
  }
  else if (lookahead == TK_PAREN_L)
  {
    return functionCall(buffer, id);
  }
  else
  {
    error("expected . or ( while parsing subroutine call");
    return false;
  }

  return true;
}

bool whileStatement(Buffer *buffer)
{
  match(buffer, TK_WHILE);

  emitXMLOpenTag("whileStatement");
  emitKeyword("while");

  match(buffer, TK_PAREN_L);
  emitSymbol("(");
  expression(buffer);
  match(buffer, TK_PAREN_R);
  emitSymbol(")");

  match(buffer, TK_BRACE_L);
  emitSymbol("{");
  statements(buffer);
  match(buffer, TK_BRACE_R);
  emitSymbol("}");

  emitXMLCloseTag("whileStatement");

  return true;
}

bool doStatement(Buffer *buffer)
{
  match(buffer, TK_DO);

  emitXMLOpenTag("doStatement");
  emitKeyword("do");
  subroutineCall(buffer);
  match(buffer, TK_SEMI);
  emitSymbol(";");
  emitXMLCloseTag("doStatement");

  return true;
}

bool returnStatement(Buffer *buffer)
{
  match(buffer, TK_RETURN);

  emitXMLOpenTag("returnStatement");
  emitKeyword("return");

  if (lookahead != TK_SEMI)
  {
    expression(buffer);
  }

  match(buffer, TK_SEMI);
  emitSymbol(";");
  emitXMLCloseTag("returnStatement");

  return true;
}

bool ifStatement(Buffer *buffer)
{
  match(buffer, TK_IF);

  emitXMLOpenTag("ifStatement");
  emitKeyword("if");

  match(buffer, TK_PAREN_L);
  emitSymbol("(");
  expression(buffer);
  match(buffer, TK_PAREN_R);
  emitSymbol(")");

  match(buffer, TK_BRACE_L);
  emitSymbol("{");
  statements(buffer);
  match(buffer, TK_BRACE_R);
  emitSymbol("}");

  // optional else
  if (lookahead == TK_ELSE)
  {
    match(buffer, TK_ELSE);
    emitKeyword("else");
    match(buffer, TK_BRACE_L);
    emitSymbol("{");
    statements(buffer);
    match(buffer, TK_BRACE_R);
    emitSymbol("}");
  }

  emitXMLCloseTag("ifStatement");

  return true;
}

bool letStatement(Buffer *buffer)
{
  match(buffer, TK_LET);

  emitXMLOpenTag("letStatement");
  emitKeyword("let");

  emitIdentifierSpecial(identifierBuffer, "reference");
  match(buffer, TK_IDENTIFIER);

  if (lookahead == TK_BRACKET_L)
  {
    // optional array expression
    match(buffer, TK_BRACKET_L);
    emitSymbol("[");
    expression(buffer);
    emitSymbol("]");
    match(buffer, TK_BRACKET_R);
  }

  match(buffer, TK_EQUAL);
  emitSymbol("=");
  expression(buffer);
  match(buffer, TK_SEMI);
  emitSymbol(";");

  emitXMLCloseTag("letStatement");

  return true;
}

bool statement(Buffer *buffer)
{
  if (lookahead != TK_IF && lookahead != TK_LET && lookahead != TK_WHILE && lookahead != TK_DO && lookahead != TK_RETURN)
  {
    return false;
  }

  switch (lookahead)
  {
  case TK_LET:
  {
    return letStatement(buffer);
  }
  case TK_IF:
  {
    return ifStatement(buffer);
  }
  case TK_WHILE:
  {
    return whileStatement(buffer);
  }
  case TK_DO:
  {
    return doStatement(buffer);
  }
  case TK_RETURN:
  {
    return returnStatement(buffer);
  }
  default:
    fprintf(stderr, "expected let | while | if | do | return but got %d\n", lookahead);
    error("error while parsing statement");
    break;
  }

  return true;
}

bool statements(Buffer *buffer)
{
  emitXMLOpenTag("statements");

  while (statement(buffer))
    ;

  emitXMLCloseTag("statements");

  return true;
}

bool subroutineBody(Buffer *buffer, ScopedSymbolTable *scopedSymbolTable)
{
  match(buffer, TK_BRACE_L);
  emitSubroutineBodyOpen();

  while (varDec(buffer, scopedSymbolTable))
    ;

  statements(buffer);

  match(buffer, TK_BRACE_R);
  emitSymbol("}");
  emitSubroutineBodyClose();
  return true;
}

bool classVarDec(Buffer *buffer)
{
  if (lookahead != TK_STATIC && lookahead != TK_FIELD)
  {
    return false;
  }

  // initialize an entry for one or more declarations
  ScopedSymbolTableEntry entry;

  emitXMLOpenTag("classVarDec");

  switch (lookahead)
  {
  case TK_STATIC:
  {
    match(buffer, TK_STATIC);
    emitKeyword("static");
    entry.kind = VK_STATIC;
    varDecDetails(buffer, &classSymbolTable, &entry);

    break;
  }
  case TK_FIELD:
  {
    match(buffer, TK_FIELD);
    emitKeyword("field");
    entry.kind = VK_FIELD;
    varDecDetails(buffer, &classSymbolTable, &entry);

    break;
  }
  default:
    fprintf(stderr, "expected static | field but got %d\n", lookahead);
    error("error while parsing class variable declaration");
    return false;
  }

  emitXMLCloseTag("classVarDec");

  return true;
}

bool subroutineDec(Buffer *buffer)
{
  if (lookahead != TK_FUNCTION && lookahead != TK_CONSTRUCTOR && lookahead != TK_METHOD)
  {
    return false;
  }

  emitXMLOpenTag("subroutineDec");

  switch (lookahead)
  {
  case TK_FUNCTION:
  {
    match(buffer, TK_FUNCTION);
    emitKeyword("function");
    break;
  }
  case TK_CONSTRUCTOR:
  {
    match(buffer, TK_CONSTRUCTOR);
    emitKeyword("constructor");
    break;
  }
  case TK_METHOD:
  {
    match(buffer, TK_METHOD);
    emitKeyword("method");
    break;
  }
  default:
    fprintf(stderr, "expected function | constructor | method but got %d\n", lookahead);
    error("error while parsing subroutine declaration");
    return false;
  }

  ScopedSymbolTableEntry subroutineSymbolTableEntries[10];
  subroutineSymbolTable.entries = subroutineSymbolTableEntries;
  startSubroutine(&subroutineSymbolTable);

  voidType(buffer);
  identifierSubroutineDeclaration(buffer);
  match(buffer, TK_PAREN_L);
  emitSymbol("(");
  parameterList(buffer, &subroutineSymbolTable);
  match(buffer, TK_PAREN_R);
  emitSymbol(")");
  subroutineBody(buffer, &subroutineSymbolTable);

  emitXMLCloseTag("subroutineDec");

  return true;
}

bool class(Buffer *buffer)
{
  int tempval;

  ScopedSymbolTableEntry classSymbolTableEntries[10];
  classSymbolTable.entries = classSymbolTableEntries;

  // class className { classVarDec* subroutineDec* }

  match(buffer, TK_CLASS);

  emitClassOpen();
  emitKeyword("class");

  tempval = tokenval;
  match(buffer, TK_IDENTIFIER);
  emitIdentifierClass(symtable[tempval].lexptr, "declaration");

  match(buffer, TK_BRACE_L);
  emitSymbol("{");

  while (classVarDec(buffer))
    ;
  while (subroutineDec(buffer))
    ;

  match(buffer, TK_BRACE_R);

  emitSymbol("}");
  emitClassClose();

  return true;
}
