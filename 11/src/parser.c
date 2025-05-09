#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>

#include "buffer.h"
#include "error.h"
#include "tokenizer.h"
#include "parser.h"
#include "emitterXML.h"
#include "emitterVM.h"

Token lookahead;
int currentAddress = 0;
int currentLabel = 0;

int nextLabel()
{
  return currentLabel++;
}

const int LABEL_SIZE = 128;

#define initLabel(label) snprintf(label, sizeof(label), "%s_%d", currentFile, nextLabel());

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
bool term(Buffer *buffer);

int parse(FILE *file)
{
  // we setbuf because we want printf to
  // output before stuff like segfaults happen
  setbuf(stdout, NULL);

  FILE *nullOut = fopen("/dev/null", "w");
  initEmitterVM(stdout);
  initEmitterXML(stdout);

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

void defineVariable(Buffer *buffer, ScopedSymbolTable *table, ScopedSymbolTableEntry *entry)
{
  defineScopedSymbol(table, entry->name, entry->type, entry->kind);
}

void nameVariable(Buffer *buffer, ScopedSymbolTable *table, ScopedSymbolTableEntry *entry)
{
  entry->name = &symtable[tokenval];
}

bool additionalVarName(Buffer *buffer, ScopedSymbolTable *table, ScopedSymbolTableEntry *entry)
{
  if (lookahead != TK_COMMA)
  {
    return false;
  }

  match(buffer, TK_COMMA);
  emitSymbol(",");

  nameVariable(buffer, table, entry);
  defineVariable(buffer, table, entry);
  match(buffer, TK_IDENTIFIER);
  emitVariableDefinitionXML(identifierBuffer);

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
bool nonVoidReturnType(Buffer *buffer)
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

bool returnType(Buffer *buffer)
{
  if (lookahead == TK_VOID)
  {
    match(buffer, TK_VOID);
    emitKeyword("void");
    return true;
  }

  return nonVoidReturnType(buffer);
}

bool parameter(Buffer *buffer, ScopedSymbolTable *table)
{
  ScopedSymbolTableEntry entry;
  entry.kind = VK_ARG;

  varType(buffer, &entry);
  nameVariable(buffer, table, &entry);
  defineVariable(buffer, table, &entry);
  match(buffer, TK_IDENTIFIER);
  emitVariableDefinitionXML(identifierBuffer);

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
  nameVariable(buffer, table, entry);
  defineVariable(buffer, table, entry);
  match(buffer, TK_IDENTIFIER);
  emitVariableDefinitionXML(identifierBuffer);

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
    term(buffer);
    emitOperation(EM_NOT);
    break;
  }
  case TK_MINUS:
  {
    match(buffer, TK_MINUS);
    emitSymbol("-");
    term(buffer);
    emitOperation(EM_NEG);
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
    break;
  }
  case TK_INTEGER:
  {
    // emit first because tokenval gets clobbered
    emitTermInteger(tokenval);
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
    expression(buffer);
    match(buffer, TK_PAREN_R);
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
    emitTermBool(true);
    emitKeyword("true");
    break;
  }
  case TK_FALSE:
  {
    match(buffer, TK_FALSE);
    emitTermBool(false);
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
    emitThisReference();
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
      emitVariableReferenceXML(id);

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
      ScopedSymbolTableEntry *entry = getIndexFromGlobalTables(id);
      emitVariableReference(entry);
      emitVariableReferenceXML(id);
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

  // we must output the next term before the op
  switch (lookahead)
  {
  case TK_PLUS:
  {
    match(buffer, TK_PLUS);
    term(buffer);
    emitOperation(EM_ADD);
    break;
  }
  case TK_MINUS:
  {
    match(buffer, TK_MINUS);
    term(buffer);
    emitOperation(EM_SUB);
    break;
  }
  case TK_ASTERISK:
  {
    match(buffer, TK_ASTERISK);
    term(buffer);
    emitOperation(EM_MUL);
    break;
  }
  case TK_SLASH:
  {
    match(buffer, TK_SLASH);
    term(buffer);
    emitOperation(EM_DIV);
    break;
  }
  case TK_AMP:
  {
    match(buffer, TK_AMP);
    term(buffer);
    emitOperation(EM_AND);
    break;
  }
  case TK_BAR:
  {
    match(buffer, TK_BAR);
    term(buffer);
    emitOperation(EM_OR);
    break;
  }
  case TK_ANGLE_BRACKET_L:
  {
    match(buffer, TK_ANGLE_BRACKET_L);
    term(buffer);
    emitOperation(EM_LT);
    break;
  }
  case TK_ANGLE_BRACKET_R:
  {
    match(buffer, TK_ANGLE_BRACKET_R);
    term(buffer);
    emitOperation(EM_GT);
    break;
  }
  case TK_EQUAL:
  {
    match(buffer, TK_EQUAL);
    term(buffer);
    emitOperation(EM_EQ);
    break;
  }
  default:
    fprintf(stderr, "expected binary operator got %d\n", lookahead);
    error("error while parsing op");
    return false;
  }

  return true;
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

int expressionList(Buffer *buffer, bool isEmpty)
{
  emitXMLOpenTag("expressionList");

  int count = 0; // the number of expressions we encounter

  // expression and term have no way to express "emptiness"
  // so the caller will need to have the context to know
  // whether this list terminates immediately or not
  // (based on the lookahead)
  if (!isEmpty)
  {
    expression(buffer);
    count++;
    while (additionalExpressions(buffer))
    {
      count++;
    }
  }

  emitXMLCloseTag("expressionList");

  return count;
}

bool functionCall(Buffer *buffer, char *subroutineName)
{
  // subroutineName ( expressionList )
  emitIdentifierSubroutine(subroutineName, "reference");
  match(buffer, TK_PAREN_L);
  emitSymbol("(");

  // needs to return how many expressions it counted
  int argc = expressionList(buffer, lookahead == TK_PAREN_R);

  // call function n
  emitFunctionCall(subroutineName, argc);

  match(buffer, TK_PAREN_R);
  emitSymbol(")");

  return true;
}

bool methodCall(Buffer *buffer, char *objectName)
{
  // ( className | varName ) . subroutineName ( expressionList )
  match(buffer, TK_DOT);
  // (className | varName) . subroutineName ( expressionList )
  char *id2 = symtable[tokenval].lexptr;

  match(buffer, TK_IDENTIFIER);
  match(buffer, TK_PAREN_L);

  // needs to return how many expressions it counted
  int argc = expressionList(buffer, lookahead == TK_PAREN_R);

  // call Class.method n
  emitMethodCall(objectName, id2, argc);

  match(buffer, TK_PAREN_R);

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
  char loop[LABEL_SIZE];
  initLabel(loop);

  char done[LABEL_SIZE];
  initLabel(done);

  /**
   * // need to keep track of these indices
   * // need the name of the current file
   * label compilationUnit_0
   *   push condition
   *   not
   *   if-goto compilationUnit_1
   *
   *   statements
   *
   *   goto compilationUnit_0
   * label compilationUnit_1
   */
  match(buffer, TK_WHILE);

  emitXMLOpenTag("whileStatement");
  emitKeyword("while");
  emitLabel(loop);

  match(buffer, TK_PAREN_L);
  emitSymbol("(");
  expression(buffer);
  // not
  // if-goto compilationUnit_1
  emitWhileLoopTest(done);
  match(buffer, TK_PAREN_R);
  emitSymbol(")");

  match(buffer, TK_BRACE_L);
  emitSymbol("{");
  statements(buffer);
  // goto compilationUnit_0
  emitWhileLoopLoop(loop); // loop
  emitLabel(done);

  match(buffer, TK_BRACE_R);
  emitSymbol("}");

  emitXMLCloseTag("whileStatement");

  return true;
}

bool doStatement(Buffer *buffer)
{
  match(buffer, TK_DO);

  emitXMLOpenTag("doStatement");
  subroutineCall(buffer);

  // the do statement is used to invoke void functions
  // so we have to clean up the stack after the return
  emitVoidFunctionCleanup();
  match(buffer, TK_SEMI);
  emitXMLCloseTag("doStatement");

  return true;
}

bool returnStatement(Buffer *buffer)
{
  match(buffer, TK_RETURN);

  emitXMLOpenTag("returnStatement");

  if (lookahead != TK_SEMI)
  {
    expression(buffer);
  }
  else
  {
    emitVoidReturnValue();
  }

  emitReturn();

  match(buffer, TK_SEMI);
  emitXMLCloseTag("returnStatement");

  return true;
}

bool ifStatement(Buffer *buffer)
{
  /**
   * expression
   * not
   * if-goto ELSE
   *
   * statements
   *
   * goto SKIP
   * label ELSE
   *
   * statements // possibly empty
   *
   * label SKIP
   */


  char elseBlock[LABEL_SIZE];
  initLabel(elseBlock);
  char done[LABEL_SIZE];
  initLabel(done);

  match(buffer, TK_IF);

  emitXMLOpenTag("ifStatement");
  emitKeyword("if");

  match(buffer, TK_PAREN_L);
  emitSymbol("(");
  expression(buffer);
  // not, if-goto
  emitIfCondition(elseBlock);
  match(buffer, TK_PAREN_R);
  emitSymbol(")");

  match(buffer, TK_BRACE_L);
  emitSymbol("{");
  statements(buffer);
  // jump to skip
  emitIfSkip(done);
  match(buffer, TK_BRACE_R);
  emitSymbol("}");

  // optional else
  if (lookahead == TK_ELSE)
  {
    match(buffer, TK_ELSE);
    emitKeyword("else");
    match(buffer, TK_BRACE_L);
    emitSymbol("{");
    emitLabel(elseBlock);
    statements(buffer);
    // just fall through to the done label
    match(buffer, TK_BRACE_R);
    emitSymbol("}");
  } else {
    // we didn't encounter an else, so we will
    // not emit XML for that... but we will
    // emit the vm code for the empty else to keep
    // things simple
    emitLabel(elseBlock);
  }

  emitLabel(done);

  emitXMLCloseTag("ifStatement");

  return true;
}

bool letStatement(Buffer *buffer)
{
  match(buffer, TK_LET);

  emitXMLOpenTag("letStatement");
  emitKeyword("let");

  ScopedSymbolTableEntry *entry = getIndexFromGlobalTables(identifierBuffer);

  if (!entry)
  {
    fprintf(stderr, "tried to assign to %s\n", identifierBuffer);
    error("while parsing let statement");
  }

  emitVariableReferenceXML(identifierBuffer);
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

  // pop into the variable
  emitVariableAssignment(entry);

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

bool subroutineBody(Buffer *buffer, ScopedSymbolTable *scopedSymbolTable, char *subroutineName)
{
  match(buffer, TK_BRACE_L);
  emitSubroutineBodyOpen();

  // here we should just build up the symbol table
  // and emit nothing
  while (varDec(buffer, scopedSymbolTable))
    ;

  emitFunctionDeclaration(currentFile, subroutineName, varCount(scopedSymbolTable, VK_VAR));
  // if this is a constructor we have to...
  statements(buffer);

  /** a return is just a statement, if we see
   * return; then the code gen will push constant 0
   * if we see return expresssion; then code gen will
   * push the result of that expression...
   * so don't worry about it ;) */

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

  ScopedSymbolTableEntry subroutineSymbolTableEntries[10];
  subroutineSymbolTable.entries = subroutineSymbolTableEntries;
  startSubroutine(&subroutineSymbolTable);

  switch (lookahead)
  {
  case TK_FUNCTION:
  {
    match(buffer, TK_FUNCTION);
    emitKeyword("function");
    returnType(buffer);
    char *subroutineName = symtable[tokenval].lexptr;
    match(buffer, TK_IDENTIFIER);
    match(buffer, TK_PAREN_L);
    emitSymbol("(");
    parameterList(buffer, &subroutineSymbolTable);
    match(buffer, TK_PAREN_R);
    emitSymbol(")");
    match(buffer, TK_BRACE_L);
    emitSubroutineBodyOpen();

    // here we should just build up the symbol table
    // and emit nothing
    while (varDec(buffer, &subroutineSymbolTable))
      ;

    emitFunctionDeclaration(currentFile, subroutineName, varCount(&subroutineSymbolTable, VK_VAR));
    statements(buffer);

    /** a return is just a statement, if we see
     * return; then the code gen will push constant 0
     * if we see return expresssion; then code gen will
     * push the result of that expression...
     * so don't worry about it ;) */

    match(buffer, TK_BRACE_R);
    emitSymbol("}");
    emitSubroutineBodyClose();

    break;
  }
  case TK_CONSTRUCTOR:
  {
    match(buffer, TK_CONSTRUCTOR);
    emitKeyword("constructor");
    returnType(buffer);
    emitIdentifierSubroutine(identifierBuffer, "declaration");
    char *subroutineName = symtable[tokenval].lexptr;
    match(buffer, TK_IDENTIFIER);
    match(buffer, TK_PAREN_L);
    emitSymbol("(");
    parameterList(buffer, &subroutineSymbolTable);
    match(buffer, TK_PAREN_R);
    emitSymbol(")");
    match(buffer, TK_BRACE_L);
    emitSubroutineBodyOpen();

    // here we should just build up the symbol table
    // and emit nothing
    while (varDec(buffer, &subroutineSymbolTable))
      ;

    emitFunctionDeclaration(currentFile, subroutineName, varCount(&subroutineSymbolTable, VK_VAR));

    emitAllocateInstance(&classSymbolTable);

    /** jack requires constructors to have return this; at the end */
    statements(buffer);

    match(buffer, TK_BRACE_R);
    emitSymbol("}");
    emitSubroutineBodyClose();

    break;
  }
  case TK_METHOD:
  {
    // methods need to
    // push argument 0
    // pop pointer 0
    match(buffer, TK_METHOD);
    emitKeyword("method");

    returnType(buffer);
    emitIdentifierSubroutine(identifierBuffer, "declaration");
    char *subroutineName = symtable[tokenval].lexptr;
    match(buffer, TK_IDENTIFIER);
    match(buffer, TK_PAREN_L);
    emitSymbol("(");
    parameterList(buffer, &subroutineSymbolTable);
    match(buffer, TK_PAREN_R);
    emitSymbol(")");
    match(buffer, TK_BRACE_L);
    emitSubroutineBodyOpen();

    // here we should just build up the symbol table
    // and emit nothing
    while (varDec(buffer, &subroutineSymbolTable))
      ;

    emitFunctionDeclaration(currentFile, subroutineName, varCount(&subroutineSymbolTable, VK_VAR));
    // if this is a constructor we have to...
    statements(buffer);

    /** a return is just a statement, if we see
     * return; then the code gen will push constant 0
     * if we see return expresssion; then code gen will
     * push the result of that expression...
     * so don't worry about it ;) */

    match(buffer, TK_BRACE_R);
    emitSymbol("}");
    emitSubroutineBodyClose();

    break;
  }
  default:
    fprintf(stderr, "expected function | constructor | method but got %d\n", lookahead);
    error("error while parsing subroutine declaration");
    return false;
  }

  emitXMLCloseTag("subroutineDec");

  return true;
}

bool class(Buffer *buffer)
{
  int tempval;
  currentLabel = 0;

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
