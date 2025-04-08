// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
// The algorithm is based on repetitive addition.

@R2
M=0 // initialize accumulator

@i
M=0

(LOOP)

@i
D=M
@R0
D=D-M
// loop while i < n
@DONE
D;JGE

// accumulate
@R1
D=M
@R2
M=D+M

@i
M=M+1 // increment i

@LOOP
0;JEQ

// loop forever
(DONE)
@DONE
0;JEQ