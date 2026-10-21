; Runtime library for compiled MICC programs.
; Appended to the generated assembly when the program uses '*', '/' or '%'.
;
; mul(a, b): repeated addition. result = sign(b) * (|b| * a) via |b|
;            additions; overflow wraps mod 2^32 (C-like).
; div(a, b): repeated subtraction with truncation toward zero (C-style):
;            q = sign(a) xor sign(b) * (|a| // |b|). Division by zero loops
;            forever (same as C UB).
; mod(a, b): remainder, sign of the dividend (C-style): |a| % |b|, then
;            negated if a < 0.
;
; Slots: params a=1, b=2; declared locals from 3. No pool usage, no calls.
; Label names are per-method, so they cannot collide with codegen labels.

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

.method div
.params 3
.locals 4
    bipush 0
    istore 3                ; q = 0
    bipush 0
    istore 6                ; neg = 0
    iload 1
    iflt Ldiv_nega
    goto Ldiv_setrem
Ldiv_nega:
    bipush 1
    istore 6                ; neg = 1
    iload 1
    bipush 0
    swap
    isub
    istore 4                ; rem = -a
    goto Ldiv_setd
Ldiv_setrem:
    iload 1
    istore 4                ; rem = a
Ldiv_setd:
    iload 2
    iflt Ldiv_negb
    goto Ldiv_setd2
Ldiv_negb:
    iload 6
    ifeq Ldiv_negb_set
    bipush 0
    istore 6                ; neg = 0 (flip)
    goto Ldiv_negb_abs
Ldiv_negb_set:
    bipush 1
    istore 6                ; neg = 1 (flip)
Ldiv_negb_abs:
    iload 2
    bipush 0
    swap
    isub
    istore 5                ; d = -b
    goto Ldiv_loop
Ldiv_setd2:
    iload 2
    istore 5                ; d = b
Ldiv_loop:
    iload 4
    iload 5
    isub
    iflt Ldiv_sign          ; rem < d -> done
    iload 4
    iload 5
    isub
    istore 4                ; rem -= d
    iinc 3, 1               ; q += 1
    goto Ldiv_loop
Ldiv_sign:
    iload 6
    ifeq Ldiv_ret
    iload 3
    bipush 0
    swap
    isub
    istore 3                ; q = -q
Ldiv_ret:
    iload 3
    ireturn

.method mod
.params 3
.locals 3
    bipush 0
    istore 5                ; neg = 0
    iload 1
    iflt Lmod_nega
    goto Lmod_setrem
Lmod_nega:
    bipush 1
    istore 5                ; neg = 1
    iload 1
    bipush 0
    swap
    isub
    istore 3                ; rem = -a
    goto Lmod_setd
Lmod_setrem:
    iload 1
    istore 3                ; rem = a
Lmod_setd:
    iload 2
    iflt Lmod_negb
    goto Lmod_setd2
Lmod_negb:
    iload 2
    bipush 0
    swap
    isub
    istore 4                ; d = -b
    goto Lmod_loop
Lmod_setd2:
    iload 2
    istore 4                ; d = b
Lmod_loop:
    iload 3
    iload 4
    isub
    iflt Lmod_sign          ; rem < d -> done
    iload 3
    iload 4
    isub
    istore 3                ; rem -= d
    goto Lmod_loop
Lmod_sign:
    iload 5
    ifeq Lmod_ret
    iload 3
    bipush 0
    swap
    isub
    istore 3                ; rem = -rem
Lmod_ret:
    iload 3
    ireturn
