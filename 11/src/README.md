# HACK Assembler

## Hack Tokens

| Token     | Symbol        | Meaning                                                    |
| --------- | ------------- | ---------------------------------------------------------- |
| TK_AT     | `@`           | begining of an A-instruction, followed by a label or value |
| TK_NUMBER | `[0-9]+`      | numeric constants for @ instructions                       |
| TK_ONE    | `1`           | for C instructions                                         |
| TK_ZERO   | `0`           | for C instructions                                         |
| TK_PLUS   | `+`           | add something                                              |
| TK_MINUS  | `-`           | subtraction, or it could be a negative number              |
| TK_BANG   | `!`           | negation                                                   |
| TK_AMP    | `&`           | logical and                                                |
| TK_BAR     | `\|`          | logical or                                                 |
| TK_EQUAL  | `=`           | indicates the destination                                  |
| TK_SEMI   | `;`           | a jump instruction follows                                 |
| TK_A      | `A`           | the currently loaded address                               |
| TK_D      | `D`           | the current data                                           |
| TK_M      | `M`           | the value at the address pointed to by A                   |
| TK_JGT    | `JGT`         | jump instruction                                           |
| TK_JEQ    | `JEQ`         | jump instruction                                           |
| TK_JGE    | `JGE`         | jump instruction                                           |
| TK_JLT    | `JLT`         | jump instruction                                           |
| TK_JNE    | `JNE`         | jump instruction                                           |
| TK_JLE    | `JLE`         | jump instruction                                           |

## Grammar

The initial restricted grammar with no symbols

```
statement := A-Instruction | C-Instruction
A-Instruction := @ TK_NUMBER
C-Instruction := dest TK_EQUAL comp jump | comp jump

comp := unary_exp | binary_exp

unary_exp := constant | unary_op TK_ONE | unary_op left_operand
constant := TK_ONE | TK_ZERO
unary_op := TK_MINUS | TK_BANG | e
left_operand := TK_A | TK_M | TK_D

binary_exp := left_operand binary_op right_operand | left_operand

binary_op := TK_PLUS | TK_MINUS | TK_AND | TK_OR
right_operand := TK_ONE | left_operand

dest := m_dest | d_dest | a_dest | null
m_dest := TK_M
d_dest := TK_D | TK_D TK_M
a_dest := TK_A | TK_AM | ad_dest
ad_dest := TK_AD | TK_ADM

jump := TK_SEMI jump_type | null
jump_type := TK_JGT | TK_JEQ | TK_JGE | TK_JLT | TK_JNE | TK_JLE
```

# NEXTSTEPS

[] implement jumps
[] implement comments
[] add symbols to the grammar
[] implement symbols