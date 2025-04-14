#ifndef PARSER_H
#define PARSER_H

#include "buffer.h"
#include "tokenizer.h"
#include <stdbool.h>
#include "ir.h"

// will allocate its own buffer
int parse(FILE *infile);

bool statement(Buffer *buffer, IR *ir);
bool pop(Buffer *buffer);
bool push(Buffer *buffer);

#endif