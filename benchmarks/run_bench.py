"""Benchmark suite for the micsim machine.

Run:  python benchmarks/run_bench.py [path/to/micsim.exe]

Compiles, assembles and runs every benchmark on the simulator, verifies
each result word, and reports cycles per benchmark plus overall totals
(TOTAL (micro) = the 9 micro-benchmarks, TOTAL (all) = + the 2 realistic
programs). Also sanity-scans the assembled binaries for opcode coverage.

Exit code 0 when every benchmark produced its expected result and every
opcode was seen at least once.
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
import asm as assembler  # noqa: E402

MICSIM = sys.argv[1] if len(sys.argv) > 1 else \
    os.path.join(ROOT, 'x64', 'Debug', 'micsim.exe')

# (name, source relative to ROOT, extra micsim args, expected word 0)
MICRO = [
    ('addsub',   'benchmarks/addsub.micc',   [],              12492500),
    ('bitwise',  'benchmarks/bitwise.micc',  [],              4294967295),
    ('muldiv',   'benchmarks/muldiv.micc',   [],              68097),
    ('branches', 'benchmarks/branches.micc', [],              2602),
    ('calls',    'benchmarks/calls.micc',    [],              4294967256),
    ('locals',   'benchmarks/locals.micc',   ['-d', '2048'],  76500),
    ('consts',   'benchmarks/consts.micc',   [],              318750),
    ('shifts',   'benchmarks/shifts.micc',   [],              5181500),
    ('incr',     'benchmarks/incr.micc',     [],              8000),
]
REALISTIC = [
    ('fib_rec', 'programs/fib_rec.micc',     [],              55),
    ('collatz', 'programs/collatz.micc',     [],              111),
]

# Opcode coverage target: every opcode except nop (excluded by decision).
# Byte-level scan over the assembled programs -- a sanity signal; operand
# bytes can in principle fake an opcode, the design matrix is the contract.
COVERAGE = {
    'bipush': 0x10, 'ldc_w': 0x13, 'iload': 0x17, 'istore': 0x36,
    'pop': 0x57, 'dup': 0x5A, 'swap': 0x5C, 'iadd': 0x62, 'isub': 0x65,
    'iand': 0x7E, 'ior': 0x81, 'iinc': 0x84, 'ifeq': 0x97, 'iflt': 0x9B,
    'if_icmpeq': 0x9F, 'goto': 0xA6, 'ireturn': 0xAC,
    'invokevirtual': 0xB4, 'ishr': 0x1C, 'wide': 0xCA, 'halt': 0xFF,
}

scratch = tempfile.mkdtemp(prefix='mic_bench_')


def run_one(name, relpath, extra_args, expected):
    """Compile -> assemble -> run -> verify. Returns (row, program bytes)."""
    with open(os.path.join(ROOT, relpath), 'r', encoding='utf-8') as f:
        source = f.read()
    try:
        asm_text = micc.compile_to_asm(source)
        program, constants = assembler.assemble(asm_text)
    except Exception as e:
        return {'name': name, 'cycles': None,
                'result': 'FAIL (%s: %s)' % (type(e).__name__, e)}, b''
    prog_path = os.path.join(scratch, name + '.bin')
    const_path = os.path.join(scratch, name + '.const.bin')
    out_path = os.path.join(scratch, name + '.out.bin')
    with open(prog_path, 'wb') as f:
        f.write(program)
    with open(const_path, 'wb') as f:
        f.write(constants)
    cmd = [MICSIM, '-i', prog_path, '-k', const_path, '-o', out_path] + \
        extra_args
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        return {'name': name, 'cycles': None,
                'result': 'FAIL (micsim exit %d)' % proc.returncode}, program
    m = re.search(r'Cycles completed:\s*(\d+)', proc.stdout)
    if not m:
        return {'name': name, 'cycles': None,
                'result': 'FAIL (no cycle count)'}, program
    cycles = int(m.group(1))
    stalls = None
    ms = re.search(r'stall:\s*(\d+)', proc.stdout)
    if ms:
        stalls = int(ms.group(1))
    with open(out_path, 'rb') as f:
        word0 = struct.unpack('<I', f.read(4))[0]
    if word0 != expected:
        return {'name': name, 'cycles': cycles,
                'result': 'FAIL (word0=%u, expected %u)'
                % (word0, expected)}, program
    return {'name': name, 'cycles': cycles, 'stalls': stalls,
            'result': 'ok'}, program


def main():
    rows = []
    all_bytes = b''
    failed = False
    for name, rel, extra, exp in MICRO + REALISTIC:
        row, program = run_one(name, rel, extra, exp)
        rows.append(row)
        all_bytes += program
        if row['result'] != 'ok':
            failed = True

    print('name         cycles       stalls      result')
    for row in rows:
        print('%-12s %-12s %-11s %s'
              % (row['name'], row['cycles'],
                 row['stalls'] if row['stalls'] is not None else '-',
                 row['result']))
    total_micro = sum(r['cycles'] for r in rows[:len(MICRO)])
    total_all = sum(r['cycles'] for r in rows)
    total_stalls = sum(r['stalls'] for r in rows if r['stalls'])
    print('TOTAL (micro) %d' % total_micro)
    print('TOTAL (all)   %d' % total_all)
    print('TOTAL (stall) %d' % total_stalls)

    missing = [name for name, byte in COVERAGE.items()
               if all_bytes.count(byte) == 0]
    if missing:
        failed = True
        print('coverage: MISSING opcodes: %s' % ', '.join(missing))
    else:
        print('coverage: %d/%d opcodes seen (sanity scan)'
              % (len(COVERAGE), len(COVERAGE)))

    print('[%s] benchmark suite' % ('FAIL' if failed else 'PASS'))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
