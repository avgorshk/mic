# mic

A simple micro-architecture simulator and its toolchain: a microprogrammed
machine (fixed datapath + writable control memory) executing a small
integer instruction set, plus the MICC language and the compiler +
assembler that target it.

```
MICC source (.micc)  ->  micc.py  ->  assembly (.asm)  ->  asm.py
    ->  program.bin + constants.bin  ->  micsim.exe  ->  output.bin
```

## 1. The machine (`micsim/`)

- Datapath: one ALU, an H register, a B-bus multiplexer, 9 C-bus
  write-enabled registers (MAR, MDR, PC, MBR, SP, LV, CPP, TOS, OPC),
  all in `mic.h`; the ALU (`alu.h`) computes AND/OR/INVB/SUM with
  `inva`/`inc` for subtraction and `sll8`/`sra1` shifts.
- Control: 512-entry control memory (`control_memory.h`) where every
  instruction's micro-routine lives; dispatch is `JMPC`: the fetched
  opcode byte IS the microcode address.
- Memory: one flat array split into three segments, byte-addressed for
  code (PC), word-addressed (32-bit LE) for constants (CPP) and data
  (LV/SP/MAR). Defaults 1 KB each, resizable with `-p/-c/-d`.

### Build

```
"C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\MSBuild.exe" mic.sln -restore -v:m -nologo -p:Configuration=Debug -p:Platform=x64
```

### Run

```
micsim.exe -i program.bin -o output.bin [-k constants.bin] [-p N] [-c N] [-d N] [--max-cycles N]
```

The simulator starts at `program[0]`, runs microcode cycles until `halt`,
and dumps the data segment as 32-bit little-endian words to `output.bin`.

## 2. Instruction set

Opcode byte == microcode address. Branch offsets are 16-bit signed,
relative to the branch instruction itself. Locals: `iload k` reads word
`LV + k`; called functions must not use slot 0 (the OBJREF slot), main may.

| mnemonic | opcode | operands | effect |
|---|---|---|---|
| nop | 0x00 | - | |
| bipush | 0x10 | 1B signed | push 8-bit sign-extended |
| ldc_w | 0x13 | 2B pool index | push constants[index] |
| iload / istore | 0x17 / 0x36 | 1B index | push / pop local k |
| pop | 0x57 | - | discard top |
| dup | 0x5A | - | push copy of TOS |
| swap | 0x5C | - | swap top two |
| iadd / isub | 0x62 / 0x65 | - | arithmetic |
| iand / ior | 0x7E / 0x81 | - | bitwise |
| iinc | 0x84 | 1B index, 1B signed | local += c |
| ifeq / iflt | 0x97 / 0x9B | 2B signed offset | pop; branch if == 0 / < 0 |
| if_icmpeq | 0x9F | 2B signed offset | pop 2; branch if equal |
| goto | 0xA6 | 2B signed offset | branch |
| ireturn | 0xAC | - | return from method |
| ishr | 0x1C | - | TOS >>= 1 (arithmetic; microcode extension using the ALU's sra1) |
| invokevirtual | 0xB4 | 2B pool index | call method |
| wide | 0xCA | iload/istore opcode + 2B index | 16-bit local index |
| halt | 0xFF | - | stop |

### Calling convention

- Method layout: `[params:2B BE][locals:2B BE][code...]`; the pool entry
  for a method is the byte address of that header. `params` includes the
  OBJREF slot (p = 1 + argc); `locals` excludes params.
- Call: `bipush 0` (OBJREF placeholder) first, then the args, then
  `invokevirtual`. Execution resumes after the 2 index bytes.
- Frame: `[F] objref(-> savedPC slot), [F+1..F+p-1] params,
  [F+p..F+p+l-1] locals, [F+p+l] savedPC, [F+p+l+1] savedLV`.
- `ireturn` writes the return value to word F; after a call SP = F.
- Only `main` needs a prologue: for k locals emit `bipush 0` then
  `k-1 x dup` (SP must sit above the locals before any push).

## 3. Assembly language (`assembler/asm.py`)

```asm
; comment
.method main              ; MUST be the first method (its code starts at byte 0)
.locals 2                 ; main: auto-prologue bipush 0 + dup x(k-1)
bipush 42
istore 0
halt

.method twice
.params 2                 ; 1 (objref) + 1 arg
.locals 0
iload 1                   ; arg (slot 0 is the OBJREF slot)
iload 1
iadd
ireturn

.const 300                ; pool entries, in source order
.const twice
```

```
python assembler/asm.py program.asm -p program.bin -c constants.bin
```

`ldc_w` and `invokevirtual` take numeric pool indices; `invokevirtual
<name>` also works and reuses/creates the method's pool entry. The
assembler emits size warnings and a static stack-depth estimate (the
latter cannot bound recursion and prints a per-level growth note instead).

## 4. The MICC language (`compiler/micc.py`)

A C-like subset: one type (`int`), functions with parameters and return
values, recursion, locals declared before use, `if`/`else`, `while`,
blocks, expression statements.

```c
int fact(int n) {
    if (n == 0)
        return 1;
    return n * fact(n - 1);
}

int main() {
    int r = fact(5);
    return r;
}
```

Operators (tightest to loosest binding): unary `- ! ~`, `* / %`, `+ -`,
`< <= > >=`, `== !=`, `&`, `|`, `&&`, `||`. Comparison operators cannot
be chained. The result of a program is main's first declared local
(output word 0 of the simulator's data-segment dump).

```
python compiler/micc.py program.micc -o program.asm     # compile
python compiler/micc.py program.micc                     # AST dump
```

`*`, `/`, `%` lower to calls into a small runtime library
(`compiler/runtime_lib.asm`) appended automatically when used: `mul`
(shift-and-add using the `ishr` opcode, O(bit length) of the multiplier),
`div` and `mod` (repeated subtraction, C-style truncation and remainder
sign). The pre-`ishr` repeated-addition `mul` is kept in
`compiler/runtime_lib_legacy.asm`; `compiler_tests/bench_mul.py` compares
the two on real cycle counts.

The same algorithms are also written in MICC itself (`runtime/rt.micc`,
a self-checking bootstrap program); `compiler_tests/test_bootstrap.py`
proves the MICC-written and hand-written runtimes produce identical
outputs.

## 5. Testing

| suite | command | scope |
|---|---|---|
| simulator | `.\x64\Debug\micsim_unit_tests.exe` | ALU + every micro-routine (doctest) |
| assembler | `python compiler_tests/test_asm.py` | encoding goldens, errors, stack warnings |
| lexer | `python compiler_tests/test_lexer.py` | tokens, positions, comments |
| parser | `python compiler_tests/test_parser.py` | AST goldens, precedence, errors |
| codegen | `python compiler_tests/test_codegen.py` | .asm goldens, error cases |
| e2e | `python compiler_tests/test_e2e.py` | MICC -> machine, 12 programs |
| bootstrap | `python compiler_tests/test_bootstrap.py` | MICC runtime == asm runtime |
| integration | `python integration_tests/micsim_tests.py .\x64\Debug\micsim.exe` | CLI flags + full pipeline |
| benchmark | `python compiler_tests/bench_mul.py` | repeated-add vs shift-add mul |

## 6. Layout

```
micsim/               simulator (C++17, MSVC)
  micro_instructions/ one header per micro-routine
micsim_unit_tests/    doctest suites
compiler/             micc.py, lexer, parser, AST, codegen, runtime libs
assembler/            asm.py
programs/             example .asm and .micc programs
runtime/              the runtime library written in MICC (bootstrap)
compiler_tests/       python test suites + benchmark
integration_tests/    CLI + pipeline tests
```

## 7. Known limits

- 1 KB default segments; deep recursion costs ~5-9 words per level in a
  256-word data segment (grow with `-d`, watch the assembler's warning).
- Division by zero loops forever (C-style UB); use `--max-cycles`.
- Branch offsets are int16, plain local indices 0..255 (`wide` beyond),
  the pool index is 16 bits.
- The simulator's word I/O is host-endian (LE on x86).
