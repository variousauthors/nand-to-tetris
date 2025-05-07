#include "symbol.h"

// two scoped symbol tables to store representation details

void emitXMLOpenTag(char* tag);
void emitXMLCloseTag(char* tag);
void emitXMLSelfClosingTag(char* tag);
void emitXMLPrimitive(char* tag, char* value);
void emitXMLPrimitiveInteger(char* tag, int value);

void emitClassOpen ();
void emitClassClose ();
void emitKeyword(char *keyword);
void emitIdentifier(char *identifier, char *mode);
void emitIdentifierSubroutine(char *identifier, char *mode);
void emitIdentifierClass(char *identifier, char *mode);
void emitSymbol(char *symbol);
void emitSubroutineBodyOpen ();
void emitSubroutineBodyClose ();
void emitVarDecOpen ();
void emitVarDecClose ();