#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

#define BUFFER_SIZE 256

typedef struct
{
  char *data;
  char *start;
  char *lookahead;
  FILE *file;
} Buffer;

void rollback(Buffer *buffer);
void commit(Buffer *buffer);
void initBuffer(Buffer *buffer, char* data, FILE *file);
char nextchar(Buffer *buffer);

#endif