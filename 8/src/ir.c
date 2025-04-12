
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ir.h"
#include "error.h"
#include "global.h"

/**
 * @param aSymbol should be -1 if there is no relevant symbol */
void insertIRNode(IR *ir, char *instruction, int aSymbol)
{
  if (ir->current > IR_SIZE - 1) {
    error("program too big for IR");
  }

  ir->nodes[ir->current].aSymbol = aSymbol;
  strcpy(ir->nodes[ir->current++].instruction, instruction);
}

char instructionBob[INSTRUCTION_SIZE] = "0000000000000000";

void fillAInstructionBob(uint16_t n)
{
  uint8_t i = 15;

  do
  {
    char nextchar = '0' + (n % 2);

    instructionBob[i--] = nextchar;
  } while ((n >>= 1) != 0);
}

void clearInstructionBob()
{
  for (int i = 0; i < INSTRUCTION_LENGTH; i++)
  {
    instructionBob[i] = '0';
  }

  instructionBob[INSTRUCTION_SIZE - 1] = '\0';
}


int address = 16;

void emit(IR *ir)
{
  for (int i = 0; i < ir->current; i++)
  {
    char *inst = ir->nodes[i].instruction;

    // we can probably move this back into the parser, lol
    if (inst[0] == '0')
    {
      if (ir->nodes[i].aSymbol > -1)
      {
        int p = ir->nodes[i].aSymbol;
        // ensure we have a value from the symbol table
        int value = symtable[p].value;
        char *label = symtable[p].lexptr;

        if (value < 0)
        {
          // if symbol still doesn't have a value at this point
          // we will give it a custome value

          fillAInstructionBob(address++);
        }
        else
        {
          fillAInstructionBob(value);
        }

        strcpy(ir->nodes[i].instruction, instructionBob);
        clearInstructionBob();
      }
    }

    printf("%s\n", ir->nodes[i].instruction);
  }
}
