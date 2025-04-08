// create an array at 100
@100
D=A
@arr
M=D
// set length to 10
@10
D=A
@n
M=D

// work
@i
M=0
(LOOP)
// if n = i we are done
@n
D=M
@i
D=D-M // n - i
@DONE
D;JEQ

// set arr[i] = -1
@arr
D=M
@i
A=D+M // A = arr + i
M=-1 // M[A] = -1

// increment i
@i
M=M+1

@LOOP
0;JMP

(DONE)
@DONE
0;JMP
