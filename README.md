# mic

A simple micro-architecture simulator and its toolchain: a microprogrammed
machine (fixed datapath + writable control memory) executing a small
integer instruction set, plus the MICC language and the compiler +
assembler that target it.

```
MICC source (.micc)  ->  micc.py  ->  assembly (.asm)  ->  asm.py
    ->  program.bin + constants.bin  ->  micsim.exe  ->  output.bin
```

## Getting started

A complete walkthrough: clone the repo, build the simulator, take a
known algorithm, compile it, run it, verify the result. The example
program is the Collatz conjecture (3n+1): counting the steps for n = 27
takes the classic 111 iterations.

You need: Windows with Visual Studio (the projects use the v145
toolset), Git, and Python 3. No Python packages are required.

### Step 1: download the repo

```
git clone https://github.com/avgorshk/mic.git
cd mic
```

### Step 2: build the simulator

Either open `mic.sln` in Visual Studio and build (Ctrl+Shift+B, x64
Debug), or from the command line:

```
"C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\MSBuild.exe" mic.sln -restore -v:m -nologo -p:Configuration=Debug -p:Platform=x64
```

(If that path does not exist on your machine, open the Visual Studio
Developer Command Prompt and run `msbuild mic.sln -restore -p:Configuration=Debug -p:Platform=x64`.)

Sanity check -- the simulator's unit tests must pass:

```
.\x64\Debug\micsim_unit_tests.exe
...
[doctest] Status: SUCCESS!
```

### Step 3: take an interesting program

Create `collatz.micc` (or use the one in `programs/`):

```c
/* Collatz conjecture: steps for n to reach 1. collatz(27) = 111. */

int collatz(int n) {
    int steps = 0;
    while (n != 1) {
        if (n % 2 == 0)
            n = n / 2;
        else
            n = 3 * n + 1;
        steps = steps + 1;
    }
    return steps;
}

int main() {
    int r = collatz(27);
    return r;
}
```

This program uses `%`, `/`, `*`, comparisons, `while` and `if`/`else`,
so it exercises the whole toolchain, including the runtime library.

### Step 4: compile it

```
python compiler/micc.py collatz.micc -o collatz.asm
```

Open `collatz.asm` -- that is the assembly your program became. (Run
`python compiler/micc.py collatz.micc` without `-o` to see the AST
dump instead.)

### Step 5: assemble it

```
python assembler/asm.py collatz.asm -p program.bin -c constants.bin
program size: 431 bytes (method area included)
constants: 4 entries (16 bytes)
```

This produces `program.bin` (the code) and `constants.bin` (the
constant pool with the method pointers for `collatz`, `mul`, `div`,
`mod`). Watch the assembler's warnings, if any.

### Step 6: run it on the simulator

```
.\x64\Debug\micsim.exe -i program.bin -k constants.bin -o output.bin
Cycles completed: 5296105
Output written: output.bin
```

(About 5.3 million microcode cycles -- most of them the repeated-
subtraction `div` doing `n / 2` on values up to 9232; the simulator
still finishes in a couple of seconds. If a program ever runs away,
add `--max-cycles 10000000`.)

### Step 7: verify the result

`output.bin` is the raw data segment as 32-bit little-endian words;
word 0 is main's first local, i.e. the program's result:

```
python -c "import struct; print(struct.unpack('<I', open('output.bin','rb').read(4))[0])"
111
```

111 steps -- Collatz confirmed on a machine with no multiply
instruction. The `3 * n + 1` multiplication was executed by the
shift-and-add `mul` routine from the runtime library, itself compiled
by the toolchain.

### Next steps

- Run the full test suite: `python compiler_tests/test_e2e.py` (13
  programs, MICC to machine end to end) and the other suites listed in
  section 5.
- Try the other programs in `programs/`: factorial, gcd, fibonacci
  (recursive and iterative), nested calls, deep recursion.
- Compare the two multiplication implementations:
  `python compiler_tests/bench_mul.py`.
- Peek at the generated assembly of any program: compile with `-o` and
  read the `.asm`.

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
`<< >>`, `< <= > >=`, `== !=`, `&`, `|`, `&&`, `||`. Comparison operators
cannot be chained. `x += e` / `x -= e` are sugar for `x = x + e` /
`x = x - e`. The result of a program is main's first declared local
(output word 0 of the simulator's data-segment dump).

Shifts: constant shifts lower inline (`x << n` to `dup; iadd` x n,
`x >> n` to n x `ishr`, counts 0..31); variable counts call `shl`/`shr`
from the runtime library. `>>` is arithmetic (sign-extending).
`x = x + c` / `x += c` with c fitting int8 compiles to a single `iinc`.

```
python compiler/micc.py program.micc -o program.asm     # compile
python compiler/micc.py program.micc                     # AST dump
```

`*`, `/`, `%` and variable-count `<<`/`>>` lower to calls into a small
runtime library (`compiler/runtime_lib.asm`) appended automatically when
used: `mul` (shift-and-add using the `ishr` opcode, O(bit length) of the
multiplier), `div` and `mod` (repeated subtraction, C-style truncation
and remainder sign), `shl` and `shr` (doubling/halving loops, `shr`
arithmetic). The pre-`ishr` repeated-addition `mul` is kept in
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
| e2e | `python compiler_tests/test_e2e.py` | MICC -> machine, 14 programs |
| bootstrap | `python compiler_tests/test_bootstrap.py` | MICC runtime == asm runtime |
| integration | `python integration_tests/micsim_tests.py .\x64\Debug\micsim.exe` | CLI flags + full pipeline |
| benchmark | `python benchmarks/run_bench.py` | 11 programs: every opcode covered, cycle totals vs `benchmarks/BASELINE.md` |
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
benchmarks/           the benchmark suite + run_bench.py + BASELINE.md
compiler_tests/       python test suites + benchmark
integration_tests/    CLI + pipeline tests
```

## 7. Micro-architecture versions

Performance is tracked by re-running `python benchmarks/run_bench.py`
after each micro-architecture change (the benchmark programs themselves
never change). Cycles are the suite totals from the latest run; the
speed-up column is the baseline TOTAL (micro) divided by the version's
TOTAL (micro).

| version | commit | TOTAL (micro) | TOTAL (stall) | TOTAL (all) | speed-up vs baseline | note |
|---|---|---|---|---|---|---|
| baseline | `0f1d1f4` | 10,141,924 | 0 | 15,450,998 | 1.00x | hardware-faithful dispatch timing: JMPC sees the pre-edge MBR (WIDE takes 2 cycles). Adopted as the reference point — the pre-fix numbers (`0da40c7`) are retired: they measured the same-edge-dispatch artifact |
| a-bus | `d4b01da` | 8,881,243 | 0 | 13,461,752 | 1.14x | dual ALU buses: A-bus mux (9 sources incl. MBR signed/unsigned), B-bus loses MBR, gains H. Reworked iadd/isub/iand/ior, iload, istore, iinc: direct operands instead of H round trips + dispatch fused into the last routine cycle (iinc −1 cycle, iload −1, istore −1, iadd family −1 each) |
| ifu | `fbdcbd4` | 6,685,865 | 303,617 | 10,251,051 | 1.52x | instruction fetch unit: PC + 6-byte shift register + MBR1/MBR2 (A-bus reads are the consumption events, JMPC consumes the opcode), 4-byte background refills from an internal auto-incrementing IMAR, `write_pc` flushes + refills. All fetch bookkeeping (PC+1 + mem_fetch + latency pads) left the microcode; stall cycles are counted separately. goto 10→5, ldc_w 8→4, invokevirtual 23→18, bipush 5→3, wide_iload 8→5 |

## 8. Known limits

- 1 KB default segments; deep recursion costs ~5-9 words per level in a
  256-word data segment (grow with `-d`, watch the assembler's warning).
- Division by zero loops forever (C-style UB); use `--max-cycles`.
- Branch offsets are int16, plain local indices 0..255 (`wide` beyond),
  the pool index is 16 bits.
- The simulator's word I/O is host-endian (LE on x86).
