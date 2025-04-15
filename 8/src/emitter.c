
/** register D will get the value baseaddr + offset for the given segment */
#include <stdio.h>
#include <stdbool.h>
#include "error.h"
#include "emitter.h"
#include "global.h"
#include <sys/syslimits.h>
#include <_stdio.h>

#define TEMP_BASEADDR 5

int labelId = 0;

void emitBootstrap()
{
  printf("@256\n");
  printf("D=A\n");
  printf("@SP\n");
  printf("A=M\n");
  printf("M=D\n"); // SP = 256

  emitCall("Sys.init", "Bootstrap", 0);
  //emitGoto("Sys.init");
  // emitCall("Sys.init", currentFile, 1);
}

void emitCall(char *functionName, char *moduleName, int nArgs)
{
  // generate the label
  char retLabel[PATH_MAX];
  snprintf(retLabel, sizeof(retLabel), "%s%s.%d", moduleName, "$ret", labelId++);

  // push the label (return address) to the stack
  printf("@%s\n", retLabel);
  printf("D=A\n");
  emitPushD();

  // push caller's stack frame
  // LCL, ARG, THIS, THAT
  printf("@LCL\n");
  printf("D=M\n");
  emitPushD();

  printf("@ARG\n");
  printf("D=M\n");
  emitPushD();

  printf("@THIS\n");
  printf("D=M\n");
  emitPushD();

  printf("@THAT\n");
  printf("D=M\n");
  emitPushD();

  // ARG = SP - 5 - nArgs (tokenval)
  printf("@%d\n", 5 + nArgs); // offset
  printf("D=A\n");
  printf("@SP\n");
  printf("D=M-D\n"); // SP - 5
  printf("@ARG\n");
  printf("M=D\n"); // ARG = SP - 5 - nArgs

  // LCL = SP
  printf("@SP\n");
  printf("D=M\n");
  printf("@LCL\n");
  printf("M=D\n");

  emitGoto(functionName);

  printf("(%s)\n", retLabel);
}

void emitPushTemp(int offset)
{
  // push the nth temp to the stack

  // addr = that + n
  int addr = TEMP_BASEADDR + offset;

  // *SP = addr*
  printf("@%d\n", addr);
  printf("D=M\n"); // D gets addr*
  emitPushD();
}

void emitPushBasic(char *segment, int offset)
{
  // put the nth local on the stack

  // addr = local + n
  printf("@%d\n", offset);
  printf("D=A\n");
  printf("@%s\n", segment);
  printf("D=D+M\n");

  // dereference local + n into D
  printf("A=D\n");
  printf("D=M\n");

  emitPushD();
}

void emitPushThis(int offset)
{
  emitPushBasic("THIS", offset);
}

void emitPushThat(int offset)
{
  emitPushBasic("THAT", offset);
}

void emitPushLocal(int offset)
{
  emitPushBasic("LCL", offset);
}

void emitPushArgument(int offset)
{
  emitPushBasic("ARG", offset);
}

void emitPushConstant(int value)
{
  // *SP = i
  printf("@%d\n", value);
  printf("D=A\n");

  emitPushD();
}

void emitPushPointer(int type)
{
  if (type == 0)
  {
    // we want to take the actual value of @THIS
    // and push it to the stack
    printf("@THIS\n");
    printf("D=M\n");
    emitPushD();
  }
  else if (type == 1)
  {
    printf("@THAT\n");
    printf("D=M\n");
    emitPushD();
  }
  else
  {
    error("tried to pop pointer with a value other than 0/1\n");
  }
}

void emitPushStatic(int n)
{
  // this needs to use the filename dynamically
  printf("@sample.%d\n", n);
  printf("D=M\n");

  emitPushD();
}

void emitPopTemp(int offset)
{
  // addr = temp + n
  int addr = TEMP_BASEADDR + offset;

  emitPopIntoD();

  printf("@%d\n", addr); // constant address
  printf("M=D\n");       // store at addr
}

void emitBasicPop(char *segment, int offset)
{
  // addr = this + n
  emitDGetsBasePlusOffset(segment, offset);

  printf("@R15\n"); // R13, R14, and R15 are our scratch registers
  printf("M=D\n");

  emitPopIntoD();

  printf("@R15\n");
  printf("A=M\n"); // jump to the addr
  printf("M=D\n"); // store at addr
}

void emitPopThat(int offset)
{
  emitBasicPop("THAT", offset);
}

void emitPopThis(int offset)
{
  emitBasicPop("THIS", offset);
}

void emitPopArgument(int offset)
{
  emitBasicPop("ARG", offset);
}

void emitPopLocal(int offset)
{
  emitBasicPop("LCL", offset);
}

void emitPopPointer(int type)
{
  if (type == 0)
  {
    emitPopIntoD();
    printf("@THAT\n");
    printf("M=D\n");
  }
  else if (type == 1)
  {
    emitPopIntoD();
    printf("@THIS\n");
    printf("M=D\n");
  }
  else
  {
    error("tried to pop pointer with a value other than 0/1\n");
  }
}

void emitPopStatic(int n)
{
  // this needs to use the filename dynamically
  emitPopIntoD();

  printf("@sample.%d\n", n);
  printf("M=D\n");
}

void emitGoto(char *label)
{
  printf("@%s\n", label);
  printf("0;JMP\n");
}
void emitIfGoto(char *label)
{
  // consume a value from the stack
  // do a jump if that is gt zero
  emitPopIntoD();
  printf("@%s\n", label);
  printf("D;JGT\n");
}

void emitReturn()
{
  // value at the top of the stack is the return value
  // *ARG = pop
  emitPopIntoD();
  printf("@ARG\n");
  printf("A=M\n");
  printf("M=D\n"); // *ARG = pop

  // SP = ARG + 1
  printf("@ARG\n");
  printf("D=M+1\n");
  printf("@SP\n");
  printf("M=D\n"); // SP = ARG + 1

  // restore caller context

  // endFrame = LCL
  // returnAdd = *(LCL - 5)

  printf("@LCL\n");
  printf("D=M\n");
  printf("@R13\n");
  printf("M=D-1\n"); // R13 := endFrame - 1

  // THAT
  printf("A=M\n");
  printf("D=M\n"); // dereference endFrame
  printf("@THAT\n");
  printf("M=D\n"); // THAT = *(endframe - 1)

  // THIS
  printf("@R13\n");
  printf("M=M-1\n"); // R13 := endFrame - 2

  printf("A=M\n");
  printf("D=M\n"); // dereference endFrame
  printf("@THIS\n");
  printf("M=D\n"); // THIS = *(endframe - 2)

  // ARG
  printf("@R13\n");
  printf("M=M-1\n"); // R13 := endFrame - 3

  printf("A=M\n");
  printf("D=M\n"); // dereference endFrame
  printf("@ARG\n");
  printf("M=D\n"); // ARG = *(endframe - 3)

  // LCL
  printf("@R13\n");
  printf("M=M-1\n"); // R13 := endFrame - 4

  printf("A=M\n");
  printf("D=M\n"); // dereference endFrame
  printf("@LCL\n");
  printf("M=D\n"); // LCL = *(endframe - 4)

  // finally GOTO retAddr
  printf("@R13\n");
  printf("M=M-1\n"); // R13 := endFrame - 5

  printf("A=M\n");
  printf("A=M\n");
  printf("0;JMP\n"); // goto *(endFrame - 5)
}

void emitInitLocal(int i)
{
  emitAGetsBasePlusOffset("LCL", i);
  printf("M=0\n");
}

void emitLabel(char *label)
{
  // emit a label into the asm
  printf("(%s)\n", label);
}

void emitAGetsBasePlusOffset(char *segment, int offset)
{
  printf("@%d\n", offset); // eg @7 if n = 7
  printf("D=A\n");
  printf("@%s\n", segment); // eg @LCL if seg = LCL
  printf("A=D+M\n");
}

/** register D will get the value baseaddr + offset for the given segment */
void emitDGetsBasePlusOffset(char *segment, int offset)
{
  printf("@%d\n", offset); // eg @7 if n = 7
  printf("D=A\n");
  printf("@%s\n", segment); // eg @LCL if seg = LCL
  printf("D=D+M\n");
}

void emitGetDFromStack()
{
  printf("@SP\n");
  printf("A=M\n");
  printf("D=M\n");
}

void emitWriteDToStack()
{
  printf("@SP\n");
  printf("A=M\n");
  printf("M=D\n");
}

void emitDecrementSP()
{
  printf("@SP\n");
  printf("M=M-1\n");
}

void emitIncrementSP()
{
  printf("@SP\n");
  printf("M=M+1\n");
}

void emitPushD()
{
  emitWriteDToStack();
  emitIncrementSP();
}

void emitPopIntoD()
{
  emitDecrementSP();
  emitGetDFromStack();
}

bool emitAdd()
{
  // get a value from the stack
  // get another value from the stack
  // add them, put that back on the stack

  // pop a value
  emitPopIntoD();

  // store it in R13
  printf("@R13\n");
  printf("M=D\n");

  // pop a value into D
  emitPopIntoD();

  // retrieve R13 and add
  printf("@R13\n");
  printf("D=D+M\n");

  emitPushD();

  return true;
}
bool emitSub()
{
  // pop a value
  emitPopIntoD();

  // store it in R13
  printf("@R13\n");
  printf("M=D\n");

  emitPopIntoD();

  // retrieve R13 and add
  printf("@R13\n");
  printf("D=D-M\n");

  emitPushD();

  return true;
}
bool emitNeg()
{
  error("neg\n");
  return true;
}
bool emitEq()
{
  error("eq\n");
  return true;
}
bool emitGt()
{
  error("gt\n");
  return true;
}
bool emitLt()
{
  char doneLabel[PATH_MAX];
  char ltLabel[PATH_MAX];
  snprintf(ltLabel, sizeof(ltLabel), "%s%s.%d", currentFile, "$lt_true", labelId++);
  snprintf(doneLabel, sizeof(doneLabel), "%s%s.%d", currentFile, "$lt_done", labelId++);

  emitSub(); // (a < b) => (a - b) < 0
  emitPopIntoD();
  printf("@%s\n", ltLabel);
  printf("D;JLT\n");

  printf("@0\n");
  printf("D=A\n");
  emitPushD();
  printf("@%s\n", doneLabel);
  printf("0;JMP\n");

  emitLabel(ltLabel);
  printf("@1\n");
  printf("D=A\n");
  emitPushD();

  emitLabel(doneLabel);

  return true;
}
bool emitLogicalAnd()
{
  error("logicalAnd\n");
  return true;
}
bool emitLogicalOr()
{
  error("logicalOr\n");
  return true;
}
bool emitLogicalNot()
{
  emitPopIntoD();

  // apply logical not
  printf("D=!D\n");

  emitPushD();

  return true;
}
