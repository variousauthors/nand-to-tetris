#ifndef PARSER_H
#define PARSER_H

#include "buffer.h"
#include "tokenizer.h"
#include <stdbool.h>
#include "ir.h"

// will allocate its own buffer
int parse(FILE *infile, FILE *out);

bool token(Buffer *buffer, IR *ir);

#endif