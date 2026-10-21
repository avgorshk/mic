"""End-to-end tests: MICC program -> compiler -> assembler -> micsim -> result.

Each program must store its result in main's local 0 (the first declared
variable of main), because the simulator dumps the data segment and the
harness reads word 0.

Run: python compiler_tests/test_e2e.py [path/to/micsim.exe]
"""

import os
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

PROGRAMS = [
    ('fib_rec', 55),
    ('fib_iter', 55),
    ('gcd', 12),
    ('fact', 120),
    ('sum1n', 55),
    ('cmp', 6),
    ('logic', 3),
    ('arith', 25),
    ('nested_call', 12),
    ('mul_neg', 12239),
    ('deep_rec', 820),
]

RESULTS = []


def run_program(name):
    src_path = os.path.join(ROOT, 'programs', name + '.micc')
    with open(src_path, 'r', encoding='utf-8') as f:
        source = f.read()

    asm_text = micc.compile_to_asm(source)
    program, constants = asm.assemble(asm_text)

    with tempfile.TemporaryDirectory() as tmp:
        prog_path = os.path.join(tmp, 'program.bin')
        const_path = os.path.join(tmp, 'constants.bin')
        out_path = os.path.join(tmp, 'output.bin')
        with open(prog_path, 'wb') as f:
            f.write(program)
        with open(const_path, 'wb') as f:
            f.write(constants)

        result = subprocess.run(
            [MICSIM, '-i', prog_path, '-k', const_path, '-o', out_path],
            capture_output=True, text=True)
        assert result.returncode == 0, \
            'micsim failed:\n%s\n%s' % (result.stdout, result.stderr)

        with open(out_path, 'rb') as f:
            raw = f.read()
        words = list(struct.unpack('<%dI' % (len(raw) // 4), raw))
        return words[0]


def main():
    failed = 0
    for name, expected in PROGRAMS:
        try:
            got = run_program(name)
            assert got == expected, '%s: expected %d, got %d' % (
                name, expected, got)
            RESULTS.append((name, True, ''))
        except Exception as e:
            RESULTS.append((name, False, '%s: %s' % (type(e).__name__, e)))

    for name, ok, msg in RESULTS:
        if ok:
            print('[PASS] %s' % name)
        else:
            print('[FAIL] %s: %s' % (name, msg))
            failed += 1
    print('Total: %d, failed: %d' % (len(RESULTS), failed))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
