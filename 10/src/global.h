#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define BSIZE 128
#define NONE -1
#define EOS '\0'

#define NUM 256
#define DIV 257
#define MOD 258
#define ID 259
#define DONE 260
#define LVALUE 261
#define ASSIGN 262
#define GOFALSE 263
#define LABEL 264
#define IF 265
#define THEN 266
#define WHILE 267
#define DO 268
#define BEGIN 269
#define END 270
#define GOTO 271

extern int tokenval;
extern int lineno;
extern int currentAddress;

typedef struct entry
{
  char *lexptr;
  int token;
  int value; // the memory address or line number
} Entry;

extern struct entry symtable[];

extern char* currentFile;