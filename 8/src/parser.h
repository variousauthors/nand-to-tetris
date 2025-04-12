#ifndef PARSER_H
#define PARSER_H

#include "buffer.h"
#include "tokenizer.h"
#include <stdbool.h>
#include "ir.h"

// will allocate its own buffer
int parse(FILE *infile);

bool statement(Buffer *buffer, IR *ir);
bool add(Buffer *buffer);
bool sub(Buffer *buffer);
bool neg(Buffer *buffer);
bool eq(Buffer *buffer);
bool gt(Buffer *buffer);
bool lt(Buffer *buffer);
bool logicalAnd(Buffer *buffer);
bool logicalOr(Buffer *buffer);
bool logicalNot(Buffer *buffer);
bool pop(Buffer *buffer);
bool push(Buffer *buffer);

#endif