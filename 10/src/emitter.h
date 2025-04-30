void emitXMLOpenTag(char* tag);
void emitXMLCloseTag(char* tag);
void emitXMLSelfClosingTag(char* tag);
void emitXMLPrimitive(char* tag, char* value);
void emitXMLPrimitiveInteger(char* tag, int value);

void emitClassOpen ();
void emitClassClose ();
void emitKeyword(char *keyword);
void emitIdentifier(char *identifier);
void emitSymbol(char *symbol);
void emitSubroutineBodyOpen ();
void emitSubroutineBodyClose ();
void emitVarDecOpen ();
void emitVarDecClose ();