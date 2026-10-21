; LEGACY runtime library: the pre-'ishr' repeated-addition mul.
; Kept for benchmarking against the shift-and-add version in
; runtime_lib.asm. NOT appended automatically by micc.py; use
; compiler_tests/bench_mul.py to compare the two.
;
; mul(a, b): repeated addition. result = sign(b) * (|b| * a) via |b|
;            additions; overflow wraps mod 2^32 (C-like).
; Cost: O(|b|) iterations, value-dependent.

.method mul
.params 3
.locals 2
    bipush 0
    istore 3                ; r = 0
    bipush 0
    istore 4                ; neg = 0
    iload 2
    iflt Lmul_neg
    goto Lmul_loop
Lmul_neg:
    bipush 1
    istore 4                ; neg = 1
    iload 2
    bipush 0
    swap
    isub
    istore 2                ; b = -b (counter = |b|)
Lmul_loop:
    iload 2
    ifeq Lmul_sign          ; counter == 0 -> done
    iload 3
    iload 1
    iadd
    istore 3                ; r += a
    iinc 2, -1
    goto Lmul_loop
Lmul_sign:
    iload 4
    ifeq Lmul_ret
    iload 3
    bipush 0
    swap
    isub
    istore 3                ; r = -r
Lmul_ret:
    iload 3
    ireturn
