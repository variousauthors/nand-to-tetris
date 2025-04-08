// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

@8192
D=A
@n
M=D // store the bytes to write in n

// while true
(LOOP)

// get the key
@KBD
D=M // D = RAM[KBD]

@CLEAR
D;JEQ // loop if no key down

(FILL)
@fill
M=-1
@DRAW
0;JEQ

(CLEAR)
@fill
M=0
@DRAW
0;JEQ

(DONE)
@LOOP
0;JEQ

// ---------- //

(DRAW)

// if key is down draw
/* FILL SCREEN */
@i
M=0 // i = 0 bytes

(DRAWLOOP)

// load up the address
@i
D=M
@SCREEN
D=D+A
@addr
M=D

// apply the fill color
@fill
D=M
@addr
A=M
M=D

// increment offset
@i
M=M+1

// if a < n loop
@n
D=M
@i
D=M-D
@DRAWLOOP
D;JLT

@DONE
0;JEQ

/* END DRAW */