"""Bootstrap test: the MICC-written runtime (runtime/rt.micc) must produce
identical outputs to the hand-written assembly runtime (compiler/runtime_lib.asm).

Two programs are generated from one case list:
  A: rt.micc's function definitions + main calling mul/div/mod directly
     (no '*' '/' '%' -> no hand-written runtime appended)
  B: main using the '*' '/' '%' operators (-> hand-written runtime appended)
The data-segment words of both runs must be identical, and must match the
C-semantics values computed here in Python.

Run: python compiler_tests/test_bootstrap.py [path/to/micsim.exe]
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

CASES = [
    ('mul', 3, 17),
    ('mul', -5, 12),
    ('mul', 7, -3),
    ('mul', -6, -9),
    ('mul', 0, 42),
    ('mul', 3, 4096),
    ('div', 17, 5),
    ('div', -17, 5),
    ('div', 17, -5),
    ('div', 7, 3),
    ('mod', 17, 5),
    ('mod', -17, 5),
    ('mod', 17, -5),
    ('mod', -17, -5),
    ('mod', 7, 3),
]


def c_div(a, b):
    return int(a / b)  # Python int() truncates toward zero


def c_mod(a, b):
    return a - c_div(a, b) * b


def c_expected(op, a, b):
    if op == 'mul':
        return a * b
    if op == 'div':
        return c_div(a, b)
    return c_mod(a, b)


def main_calls_source():
    lines = ['int main() {']
    for i, (op, a, b) in enumerate(CASES):
        lines.append('    int r%d = %s(%d, %d);' % (i, op, a, b))
    lines.append('    return r0;')
    lines.append('}')
    return '\n'.join(lines) + '\n'


def main_ops_source():
    op_char = {'mul': '*', 'div': '/', 'mod': '%'}
    lines = ['int main() {']
    for i, (op, a, b) in enumerate(CASES):
        lines.append('    int r%d = %d %s %d;' % (i, a, op_char[op], b))
    lines.append('    return r0;')
    lines.append('}')
    return '\n'.join(lines) + '\n'


def run_source(source):
    asm_text = micc.compile_to_asm(source)
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
        return list(struct.unpack('<%dI' % (len(raw) // 4), raw))


def to_signed(v):
    return v - 0x100000000 if v >= 0x80000000 else v


def main():
    rt_path = os.path.join(ROOT, 'runtime', 'rt.micc')
    with open(rt_path, 'r', encoding='utf-8') as f:
        rt_source = f.read()
    # the function definitions are everything before rt.micc's own main
    funcs = rt_source.split('int main()')[0]

    failed = 0
    words_a = run_source(funcs + main_calls_source())
    words_b = run_source(main_ops_source())
    n = len(CASES)

    if words_a[:n] != words_b[:n]:
        print('[FAIL] MICC runtime and asm runtime outputs differ:')
        for i, (wa, wb) in enumerate(zip(words_a, words_b)):
            if wa != wb:
                print('  case %d (%s): micc=%d asm=%d' % (
                    i, CASES[i], wa, wb))
        failed += 1
    else:
        print('[PASS] micc runtime == asm runtime (%d cases)' % n)

    for i, (op, a, b) in enumerate(CASES):
        expected = c_expected(op, a, b)
        ok = to_signed(words_a[i]) == expected \
            and to_signed(words_b[i]) == expected
        if not ok:
            print('[FAIL] case %d: %s(%d, %d) = %d/%d, expected %d' % (
                i, op, a, b, to_signed(words_a[i]), to_signed(words_b[i]),
                expected))
            failed += 1
    if failed == 0:
        print('[PASS] all %d cases match C semantics' % n)
    print('Total: %d, failed: %d' % (2, failed))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
