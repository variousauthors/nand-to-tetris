#ifndef EMITTER_XML_H
#define EMITTER_XML_H

#include "symbol.h"

// two scoped symbol tables to store representation details
void initEmitterXML(FILE *out);

void emitXMLOpenTag(char* tag);
void emitXMLCloseTag(char* tag);
void emitXMLSelfClosingTag(char* tag);
void emitXMLPrimitive(char* tag, char* value);
void emitXMLPrimitiveInteger(char* tag, int value);

void emitClassOpen ();
void emitClassClose ();
void emitKeyword(char *keyword);
void emitVariableReferenceXML(char *identifier);
void emitVariableDefinitionXML(char *identifier);
void emitIdentifierSubroutine(char *identifier, char *mode);
void emitIdentifierClass(char *identifier, char *mode);
void emitSymbol(char *symbol);
void emitSubroutineBodyOpen ();
void emitSubroutineBodyClose ();
void emitVarDecOpen ();
void emitVarDecClose ();

#endif