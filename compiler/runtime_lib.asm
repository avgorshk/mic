; Runtime library for compiled MICC programs.
; Appended to the generated assembly when the program uses '*', '/' or '%'.
;
; mul(a, b): shift-and-add (schoolbook binary multiplication) using the
;            'ishr' opcode (TOS >>= 1, arithmetic). The multiplier is held
;            non-negative, so the arithmetic shift behaves as a logical
;            shift and the loop terminates after at most 32 iterations.
;            result = sign(a) xor sign(b) * (|a| * |b|); overflow wraps
;            mod 2^32 (C-like). Cost: O(bit length of b), value-independent.
;            The pre-ishr repeated-addition version is kept in
;            runtime_lib_legacy.asm for comparison.
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
.locals 4                ; result=3, a_abs=4, b_abs=5, neg=6
    bipush 0
    istore 3             ; result = 0
    bipush 0
    istore 6             ; neg = 0
    iload 1
    iflt Lmul_nega
    goto Lmul_seta
Lmul_nega:
    bipush 1
    istore 6             ; neg = 1
    iload 1
    bipush 0
    swap
    isub
    istore 4             ; a_abs = -a
    goto Lmul_setb
Lmul_seta:
    iload 1
    istore 4             ; a_abs = a
Lmul_setb:
    iload 2
    iflt Lmul_negb
    goto Lmul_setb2
Lmul_negb:
    iload 6
    ifeq Lmul_negb_set
    bipush 0
    istore 6             ; neg = 0 (flip)
    goto Lmul_negb_abs
Lmul_negb_set:
    bipush 1
    istore 6             ; neg = 1 (flip)
Lmul_negb_abs:
    iload 2
    bipush 0
    swap
    isub
    istore 5             ; b_abs = -b
    goto Lmul_loop
Lmul_setb2:
    iload 2
    istore 5             ; b_abs = b
Lmul_loop:
    iload 5
    ifeq Lmul_sign       ; b == 0 -> done
    iload 5
    bipush 1
    iand
    ifeq Lmul_skip       ; (b & 1) == 0
    iload 3
    iload 4
    iadd
    istore 3             ; result += a
Lmul_skip:
    iload 4
    iload 4
    iadd
    istore 4             ; a <<= 1 (a + a)
    iload 5
    ishr                 ; b >>= 1
    istore 5
    goto Lmul_loop
Lmul_sign:
    iload 6
    ifeq Lmul_ret
    iload 3
    bipush 0
    swap
    isub
    istore 3             ; result = -result
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
