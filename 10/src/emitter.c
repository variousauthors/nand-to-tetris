
/** register D will get the value baseaddr + offset for the given segment */
#include <stdio.h>
#include <stdbool.h>
#include "emitter.h"
#include "global.h"
#include <sys/syslimits.h>
#include <_stdio.h>

#define TEMP_BASEADDR 5

int indent = 0;
int MAX_INDENT = 0;

void makeIndentation (char *indentation) {
  int i = 0;
  for (; i < indent && i < MAX_INDENT; i++) {
    indentation[i] = ' ';
  }

  // terminate after the last char we actually wrote
  indentation[i] = '\0';
}

void emitXMLPrimitive(char* tag, char* value) {
  char indentation[indent + 1];
  makeIndentation(indentation);

  printf("%s<%s> %s </%s>\n", indentation, tag, value, tag);
}

void emitXMLPrimitiveInteger(char* tag, int value) {
  char indentation[indent + 1];
  makeIndentation(indentation);

  printf("%s<%s> %d </%s>\n", indentation, tag, value, tag);
}

void emitXMLOpenTag(char* tag) {
  char indentation[indent + 1];
  makeIndentation(indentation);

  printf("%s<%s>\n", indentation, tag);
  indent++;
}

void emitXMLCloseTag(char* tag) {
  indent--;

  char indentation[indent + 1];
  makeIndentation(indentation);

  printf("%s</%s>\n", indentation, tag);
}

void emitXMLSelfClosingTag(char* tag) {
  char indentation[indent + 1];
  makeIndentation(indentation);

  printf("%s<%s/>\n", indentation, tag);
}
