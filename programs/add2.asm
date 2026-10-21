; add2.asm -- M0 reference program.
; main calls add2(40, 2), stores the result in local 0, halts.
; Expected result: output.bin word 0 == 42.
;
; Assembled layout (assembler reference for M1):
;   program.bin (24 bytes):
;     byte  0..1  : bipush 0        (0x10 0x00)  prologue: reserve 1 local
;     byte  2..3  : bipush 0        (0x10 0x00)  OBJREF placeholder
;     byte  4..5  : bipush 40       (0x10 0x28)  arg1
;     byte  6..7  : bipush 2        (0x10 0x02)  arg2
;     byte  8..10 : invokevirtual 0 (0xB4 0x00 0x00)
;     byte 11..12 : istore 0        (0x36 0x00)
;     byte 13     : halt            (0xFF)
;     byte 14..15 : params = 3      (0x00 0x03)  add2 header (1 objref + 2 args)
;     byte 16..17 : locals = 0      (0x00 0x00)
;     byte 18..19 : iload 1         (0x17 0x01)  arg1 (callee slot 0 = OBJREF)
;     byte 20..21 : iload 2         (0x17 0x02)  arg2
;     byte 22     : iadd            (0x62)
;     byte 23     : ireturn         (0xAC)
;   constants.bin (1 word, LE uint32):
;     word 0 : 14                    (byte address of add2's params field)
;
; Trace summary (verified against microcode):
;   main: SP=base, LV=base. Prologue push -> SP=base+1 (1 local reserved).
;         objref push -> base+2; args -> base+3, base+4. SP=base+4.
;   call:  F = base+2 (OBJREF slot); p=3, l=0.
;          savedPC slot = F+p+l = base+5, savedLV slot = base+6.
;          objref value (word F) = base+5; savedPC = addr(istore);
;          savedLV = base. After call: LV=F, SP=base+6.
;   callee: iload 1 -> word F+1 = 40; iload 2 -> word F+2 = 2; iadd -> 42.
;   return: word F = 42 (return value); PC = savedPC; LV = base; SP = F.
;   main:   istore 0 -> word base = 42 (output word 0); halt.
;
; Note: local 0 of main == data segment word 0 == output.bin word 0.

.method main
.locals 1                ; auto-prologue: bipush 0 (reserves local 0)
bipush 0                 ; OBJREF placeholder (overwritten by the call)
bipush 40                ; arg1
bipush 2                 ; arg2
invokevirtual add2
istore 0                 ; result -> main's local 0
halt

.method add2
.params 3                ; 1 (objref) + 2 args
.locals 0                ; no extra locals
iload 1                  ; arg1 (slot 0 is the OBJREF slot)
iload 2                  ; arg2
iadd
ireturn

.const add2              ; pool word 0 = byte address of add2's header
