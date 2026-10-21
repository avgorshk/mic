; fib_iter.asm -- M1 reference program: iterative fibonacci with a while loop.
; main computes fib(10) iteratively, stores the result in local 0, halts.
; Expected result: output.bin word 0 == 55.
;
; Locals of main:
;   0: result     1: a     2: b     3: count     4: t
; Loop: while count != 0: t = a + b; a = b; b = t; count -= 1
; After 10 iterations a = fib(10) = 55, b = fib(11) = 89; result = a.
;
; Exercises: backward goto, ifeq (pops the condition), iinc with a negative
; constant, and main's auto-prologue (.locals 5 -> bipush 0 + 4 x dup).

.method main
.locals 5
bipush 0
istore 1                 ; a = 0
bipush 1
istore 2                 ; b = 1
bipush 10
istore 3                 ; count = 10
loop:
iload 3
ifeq done                ; while count != 0
iload 1
iload 2
iadd
istore 4                 ; t = a + b
iload 2
istore 1                 ; a = b
iload 4
istore 2                 ; b = t
iinc 3, -1               ; count -= 1
goto loop
done:
iload 1                  ; result = a
istore 0
halt
