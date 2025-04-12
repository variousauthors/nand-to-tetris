
#define INSTRUCTION_LENGTH 16
#define INSTRUCTION_SIZE INSTRUCTION_LENGTH + 1

typedef struct
{
  char instruction[INSTRUCTION_SIZE];
  int aSymbol; // an index into the symbol table
} IRNode;

#define IR_SIZE 65536

typedef struct {
  IRNode *nodes;
  int current;
} IR;

void insertIRNode(IR *ir, char *instruction, int aSymbol);

void emit(IR *ir);