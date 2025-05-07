#include <stdio.h>
#include <stddef.h>
#include "buffer.h"

// after we trigger the buffer fil
// on encountering an EOF
// we want to turn it off so we
// do not re-trigger it after a rollback
#define STALE_EOF -2

void fillBufferL(Buffer *buffer)
{
  char *data = buffer->data;
  size_t bytesRead = fread(data, 1, BUFFER_SIZE / 2, buffer->file);
  data[bytesRead] = EOF;
}

void fillBufferR(Buffer *buffer)
{
  char *data = buffer->data;
  size_t bytesRead = fread(data + (BUFFER_SIZE / 2 + 1), 1, BUFFER_SIZE / 2, buffer->file);
  data[BUFFER_SIZE / 2 + 1 + bytesRead] = EOF;
}

void rollback(Buffer *buffer)
{
  buffer->lookahead = buffer->start;
}

/* advance the start to the lookahead */
void commit(Buffer *buffer)
{
  buffer->start = buffer->lookahead;
}

char nextchar(Buffer *buffer)
{

  // we skip stale EOFs
  if (*buffer->lookahead == STALE_EOF) {
    ptrdiff_t index = buffer->lookahead - buffer->data;

    switch (index)
    {
    case BUFFER_SIZE / 2: {
      // left side, just skip the EOF
      buffer->lookahead++;

      return *buffer->lookahead++;
    }
       
    case BUFFER_SIZE + 1: {
      // right side, set lookahead to the start
      buffer->lookahead = buffer->data;
      return *buffer->lookahead++;
    }
    default:
      // actual end of file
      return EOF;
    }
  }

  if (*buffer->lookahead == EOF)
  {
    ptrdiff_t index = buffer->lookahead - buffer->data;

    switch (index)
    {
    case BUFFER_SIZE / 2:
      // we are at the end of left half we need to fill right
      // before we nextchar
      fillBufferR(buffer);
      *buffer->lookahead = STALE_EOF;
      buffer->lookahead++;

      ptrdiff_t index = buffer->lookahead - buffer->data;

      return *buffer->lookahead++;
    case BUFFER_SIZE + 1:
      // we are at the end of right half we need to fill right
      // before we nextchar
      fillBufferL(buffer);
      *buffer->lookahead = STALE_EOF;
      buffer->lookahead = buffer->data;
      return *buffer->lookahead++;
    default:
      // actual end of file
      return EOF;
    }
  }

  // OK so now
  // @SP�r
  //   ^ ^
  // lookahead is pointing past the EOF
  // but if we rollback we will end up on the
  // left-side of the EOF
  // we could be arbitrarily far from the start
  // when we roll back we will end up on the
  // other side of the EOF and we will re-trigger it
  // 1. we could replace the EOF with another special
  // symbol indicating it has been triggered
  // and just advance over it...
  // @SP�r


  return *buffer->lookahead++;
}

void initBuffer(Buffer *buffer, char *data, FILE *file) {
  buffer->data = data;
  buffer->file = file;
  buffer->start = data;
  buffer->lookahead = data;

  // the sentinels after the first half and second half
  data[BUFFER_SIZE / 2] = EOF;
  data[BUFFER_SIZE + 1] = EOF;

  fillBufferL(buffer);
}
