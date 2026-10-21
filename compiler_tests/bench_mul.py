"""Benchmark: repeated-add vs shift-add integer multiplication.

For each multiplier b, compiles 'r = 3 * b' twice -- once with the current
shift-and-add runtime (runtime_lib.asm, uses the ishr opcode) and once with
the legacy repeated-addition version (runtime_lib_legacy.asm) -- runs both
on micsim and reports cycle counts.

Run: python compiler_tests/bench_mul.py [path/to/micsim.exe]
"""

import os
import re
import struct
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'compiler'))
sys.path.insert(0, os.path.join(ROOT, 'assembler'))

import micc  # noqa: E402
import asm  # noqa: E402

MICSIM = os.path.join(ROOT, 'x64', 'Debug', 'micsim.exe')
if len(sys.argv) > 1:
    MICSIM = sys.argv[1]

LEGACY = os.path.join(ROOT, 'compiler', 'runtime_lib_legacy.asm')
MULTIPLIERS = [8, 32, 128, 512, 2048, 8192]


def compile_with_runtime(source, runtime_path):
    codegen = micc.Codegen(micc.compile_to_ast(source))
    text = codegen.gen()
    if codegen.runtime_used:
        with open(runtime_path, 'r', encoding='utf-8') as f:
            text += '\n' + f.read()
    return text


def run_program(asm_text, b):
    program, constants = asm.assemble(asm_text)
    with tempfile.TemporaryDirectory() as tmp:
        prog_path = os.path.join(tmp, 'p.bin')
        const_path = os.path.join(tmp, 'c.bin')
        out_path = os.path.join(tmp, 'o.bin')
        with open(prog_path, 'wb') as f:
            f.write(program)
        with open(const_path, 'wb') as f:
            f.write(constants)
        result = subprocess.run(
            [MICSIM, '-i', prog_path, '-k', const_path, '-o', out_path],
            capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        with open(out_path, 'rb') as f:
            raw = f.read()
        value = struct.unpack('<%dI' % (len(raw) // 4), raw)[0]
        assert value == 3 * b, 'wrong result for b=%d: %d' % (b, value)
        m = re.search(r'Cycles completed: (\d+)', result.stdout)
        assert m, result.stdout
        return int(m.group(1))


def main():
    source_fmt = 'int main() {\n    int r = 3 * %d;\n    return r;\n}\n'
    print('%-10s %14s %14s %8s' % ('b', 'repeated-add', 'shift-add', 'speedup'))
    for b in MULTIPLIERS:
        source = source_fmt % b
        naive = run_program(compile_with_runtime(source, LEGACY), b)
        fast = run_program(micc.compile_to_asm(source), b)
        print('%-10d %14d %14d %7.1fx' % (b, naive, fast, naive / fast))
    return 0


if __name__ == '__main__':
    sys.exit(main())
