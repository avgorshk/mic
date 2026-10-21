; fib_rec.asm -- M1 reference program: naive recursive fibonacci.
; main calls fib(10), stores the result in local 0, halts.
; Expected result: output.bin word 0 == 55.
;
; Notes:
;   - fib has no extra locals: everything lives on the stack.
;   - the call protocol needs the OBJREF placeholder BELOW the argument,
;     but the argument is computed first -> bipush 0 + swap fixes the order.
;   - base case: fib(n) = n for n < 2  (tested as n - 2 < 0).

.method main
.locals 1
bipush 0                 ; OBJREF placeholder
bipush 10                ; n
invokevirtual fib
istore 0
halt

.method fib
.params 2                ; 1 (objref) + n
.locals 0
iload 1
bipush 2
isub                     ; n - 2
iflt base                ; n < 2 -> return n
; recursive case: fib(n-1) + fib(n-2)
iload 1
bipush 1
isub                     ; stack: n-1
bipush 0                 ; stack: n-1, objref
swap                     ; stack: objref, n-1
invokevirtual fib        ; stack: fib(n-1)
iload 1
bipush 2
isub                     ; stack: fib(n-1), n-2
bipush 0
swap                     ; stack: fib(n-1), objref, n-2
invokevirtual fib        ; stack: fib(n-1), fib(n-2)
iadd
ireturn
base:
iload 1
ireturn
