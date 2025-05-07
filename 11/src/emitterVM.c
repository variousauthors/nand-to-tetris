#include <stdio.h>

void emitMethodCall(char *objectName, char *methodName, int argc) {
  printf("call %s.%s %d\n", objectName, methodName, argc);
}

void emitConstant(int n) {
  printf("push constant %d\n", n);
}

void emitOperation(char op) {
  switch (op)
  {
  case '*': {
    printf("call Math.multiply 2\n");
    break;
  }
  case '+': {
    printf("sub\n");
    break;
  }
  default:
    break;
  }

}