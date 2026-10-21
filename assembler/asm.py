"""Assembler for the micsim instruction set.

Converts .asm source into program.bin + constants.bin for the micsim
MIC-1 simulator. See .hermes/plans/2026-10-20_201705-micc-micsim-compiler.md
for the ISA/ABI contract.

Usage:
    python asm.py input.asm [-p program.bin] [-c constants.bin]

Syntax:
    ; comment
    .method main          ; entry point, MUST be the first method (byte 0)
    .params n             ; callee: 1 (objref) + argc ; default 1 ; main: forbidden
    .locals n             ; callee: local slots excl. params ; default 0
                          ; main: locals 0..n-1 ; assembler auto-emits the
                          ; prologue (bipush 0, then dup x(n-1))
    .const <int>          ; constant pool entry (index assigned in source order)
    .const <method>       ; constant pool entry: byte address of method header
    label:
    bipush b              ; b in [-128, 127]
    ldc_w <index>         ; push pool[index]
    iload k | istore k    ; k in [0, 255]
    wide iload k | wide istore k   ; k in [0, 65535]
    iinc k, c             ; k in [0, 255], c in [-128, 127]
    pop dup swap iadd isub iand ior ireturn halt nop
    ifeq L | iflt L | if_icmpeq L | goto L   ; offset relative to branch address
    invokevirtual <index> ; pool[index] must be a method entry
    invokevirtual <name>  ; reuse or append the method's pool entry
"""

import argparse
import struct
import sys

PROGRAM_SEGMENT_DEFAULT = 1024   # bytes
CONSTANT_SEGMENT_DEFAULT = 1024  # bytes

OPCODES = {
    'nop': 0x00, 'bipush': 0x10, 'ldc_w': 0x13, 'iload': 0x17, 'istore': 0x36,
    'pop': 0x57, 'dup': 0x5A, 'swap': 0x5C, 'iadd': 0x62, 'isub': 0x65,
    'iand': 0x7E, 'ior': 0x81, 'iinc': 0x84, 'ifeq': 0x97, 'iflt': 0x9B,
    'if_icmpeq': 0x9F, 'goto': 0xA6, 'ireturn': 0xAC, 'invokevirtual': 0xB4,
    'wide': 0xCA, 'halt': 0xFF,
}

NO_OPERAND = {'nop', 'pop', 'dup', 'swap', 'iadd', 'isub', 'iand', 'ior',
              'ireturn', 'halt'}
BRANCHES = {'ifeq', 'iflt', 'if_icmpeq', 'goto'}


class AsmError(Exception):
    def __init__(self, line_no, msg):
        super().__init__(msg)
        self.line_no = line_no
        self.msg = msg


class Method:
    def __init__(self, name, line_no):
        self.name = name
        self.line_no = line_no
        self.params = None
        self.locals_count = 0
        self.code = []          # (line_no, 'label'|'insn', data)
        self.address = 0
        self.labels = {}


def _try_int(tok):
    try:
        return int(tok, 16) if tok.lower().startswith('0x') else int(tok)
    except ValueError:
        return None


def _parse_int(tok, line_no, what, lo, hi):
    v = _try_int(tok)
    if v is None:
        raise AsmError(line_no, "invalid integer '%s' in %s" % (tok, what))
    if not (lo <= v <= hi):
        raise AsmError(line_no, "%s %d out of range [%d, %d]" % (what, v, lo, hi))
    return v


def _check_identifier(name, line_no, what):
    if (not name
            or not (name[0].isalpha() or name[0] == '_')
            or not all(c.isalnum() or c == '_' for c in name)):
        raise AsmError(line_no, "invalid %s name '%s'" % (what, name))


def _parse(source):
    methods = []
    methods_by_name = {}
    consts = []                 # (line_no, 'int', value) | (line_no, 'method', name)
    cur = None

    for line_no, raw in enumerate(source.splitlines(), 1):
        line = raw.split(';', 1)[0].strip()
        if not line:
            continue
        toks = [t.rstrip(',') for t in line.split() if t.rstrip(',')]
        if not toks:
            continue

        head = toks[0]

        if head == '.method':
            if len(toks) != 2:
                raise AsmError(line_no, ".method expects one name")
            name = toks[1]
            _check_identifier(name, line_no, 'method')
            if name in methods_by_name:
                raise AsmError(line_no, "duplicate method '%s'" % name)
            if not methods and name != 'main':
                raise AsmError(
                    line_no, "first method must be 'main' (entry point at byte 0)")
            if methods and name == 'main':
                raise AsmError(line_no, "'main' must be the first method")
            cur = Method(name, line_no)
            methods.append(cur)
            methods_by_name[name] = cur
            continue

        elif head == '.params':
            if cur is None:
                raise AsmError(line_no, ".params outside a method")
            if len(toks) != 2:
                raise AsmError(line_no, ".params expects one number")
            if cur.name == 'main':
                raise AsmError(line_no, "main cannot have .params")
            cur.params = _parse_int(toks[1], line_no, ".params", 1, 0xFFFF)
            continue

        elif head == '.locals':
            if cur is None:
                raise AsmError(line_no, ".locals outside a method")
            if len(toks) != 2:
                raise AsmError(line_no, ".locals expects one number")
            cur.locals_count = _parse_int(toks[1], line_no, ".locals", 0, 0xFFFF)
            continue

        elif head == '.const':
            if len(toks) != 2:
                raise AsmError(line_no, ".const expects one integer or method name")
            v = _try_int(toks[1])
            if v is not None:
                if not (-0x80000000 <= v <= 0xFFFFFFFF):
                    raise AsmError(line_no, ".const value %d out of 32-bit range" % v)
                consts.append((line_no, 'int', v))
            else:
                _check_identifier(toks[1], line_no, 'method')
                consts.append((line_no, 'method', toks[1]))
            continue

        elif head.endswith(':') and len(head) > 1:
            label = head[:-1]
            _check_identifier(label, line_no, 'label')
            if cur is None:
                raise AsmError(line_no, "label outside a method")
            cur.code.append((line_no, 'label', label))
            if len(toks) > 1:
                toks = toks[1:]
            else:
                continue

        if cur is None:
            raise AsmError(line_no, "instruction outside a method")

        mnem = toks[0]

        if mnem in NO_OPERAND:
            if len(toks) != 1:
                raise AsmError(line_no, "'%s' takes no operands" % mnem)
            cur.code.append((line_no, 'insn', (mnem, None)))

        elif mnem == 'bipush':
            if len(toks) != 2:
                raise AsmError(line_no, "bipush expects one operand")
            v = _parse_int(toks[1], line_no, 'bipush', -128, 127)
            cur.code.append((line_no, 'insn', ('bipush', v)))

        elif mnem == 'ldc_w':
            if len(toks) != 2:
                raise AsmError(line_no, "ldc_w expects one pool index")
            v = _parse_int(toks[1], line_no, 'ldc_w index', 0, 0xFFFF)
            cur.code.append((line_no, 'insn', ('ldc_w', v)))

        elif mnem in ('iload', 'istore'):
            if len(toks) != 2:
                raise AsmError(line_no, "%s expects one operand" % mnem)
            v = _parse_int(toks[1], line_no, mnem, 0, 255)
            cur.code.append((line_no, 'insn', (mnem, v)))

        elif mnem == 'iinc':
            if len(toks) != 3:
                raise AsmError(line_no, "iinc expects two operands: k, c")
            k = _parse_int(toks[1], line_no, 'iinc index', 0, 255)
            c = _parse_int(toks[2], line_no, 'iinc const', -128, 127)
            cur.code.append((line_no, 'insn', ('iinc', (k, c))))

        elif mnem in BRANCHES:
            if len(toks) != 2:
                raise AsmError(line_no, "%s expects one label" % mnem)
            _check_identifier(toks[1], line_no, 'label')
            cur.code.append((line_no, 'insn', (mnem, toks[1])))

        elif mnem == 'invokevirtual':
            if len(toks) != 2:
                raise AsmError(line_no, "invokevirtual expects a pool index or method name")
            v = _try_int(toks[1])
            if v is not None:
                if not (0 <= v <= 0xFFFF):
                    raise AsmError(line_no, "invokevirtual index %d out of range" % v)
                cur.code.append((line_no, 'insn', ('invokevirtual', ('idx', v))))
            else:
                _check_identifier(toks[1], line_no, 'method')
                cur.code.append((line_no, 'insn', ('invokevirtual', ('name', toks[1]))))

        elif mnem == 'wide':
            if len(toks) != 3 or toks[1] not in ('iload', 'istore'):
                raise AsmError(line_no, "wide expects 'iload' or 'istore' and an index")
            k = _parse_int(toks[2], line_no, 'wide index', 0, 0xFFFF)
            cur.code.append((line_no, 'insn', ('wide', (toks[1], k))))

        else:
            raise AsmError(line_no, "unknown instruction '%s'" % mnem)

    if not methods:
        raise AsmError(1, "no .method defined (first method must be 'main')")
    return methods, methods_by_name, consts


def _insn_size(insn):
    mnem, operand = insn
    if mnem in NO_OPERAND:
        return 1
    if mnem in ('bipush', 'iload', 'istore'):
        return 2
    if mnem in ('ldc_w', 'invokevirtual') or mnem in BRANCHES:
        return 3
    if mnem == 'iinc':
        return 3
    if mnem == 'wide':
        return 4
    raise AssertionError("unknown mnemonic %r" % mnem)


def _layout(methods):
    offset = 0
    for m in methods:
        m.address = offset
        if m.name == 'main' and m.locals_count > 0:
            prologue = [(m.line_no, 'insn', ('bipush', 0))]
            prologue += [(m.line_no, 'insn', ('dup', None))] * (m.locals_count - 1)
            m.code = prologue + m.code
        if m.params is None:
            m.params = 1
        header = 0 if m.name == 'main' else 4  # main starts at byte 0, no header
        addr = offset + header
        for line_no, kind, data in m.code:
            if kind == 'label':
                if data in m.labels:
                    raise AsmError(line_no, "duplicate label '%s'" % data)
                m.labels[data] = addr
            else:
                addr += _insn_size(data)
        offset = addr
    return offset


def _build_pool(methods, methods_by_name, consts):
    pool = []               # ('int', value) | ('method', name)
    method_index = {}       # name -> pool index

    def add_method_ref(name, line_no):
        if name not in methods_by_name:
            raise AsmError(line_no, "unknown method '%s'" % name)
        if name not in method_index:
            method_index[name] = len(pool)
            pool.append(('method', name))

    for line_no, kind, value in consts:
        if kind == 'method':
            add_method_ref(value, line_no)
        else:
            pool.append(('int', value))

    for m in methods:
        for line_no, kind, data in m.code:
            if kind == 'insn' and data[0] == 'invokevirtual' and data[1][0] == 'name':
                add_method_ref(data[1][1], line_no)

    return pool, method_index


def _emit_program(methods, pool, method_index):
    out = bytearray()
    for m in methods:
        method_start = len(out)
        is_main = (m.name == 'main')
        if not is_main:
            out += struct.pack('>HH', m.params, m.locals_count)
        for line_no, kind, data in m.code:
            if kind == 'label':
                continue
            mnem, operand = data
            out.append(OPCODES[mnem])
            if mnem == 'bipush':
                out.append(operand & 0xFF)
            elif mnem in ('iload', 'istore'):
                out.append(operand)
            elif mnem == 'iinc':
                out.append(operand[0])
                out.append(operand[1] & 0xFF)
            elif mnem == 'ldc_w':
                if operand >= len(pool):
                    raise AsmError(line_no,
                                   "ldc_w index %d out of range (pool has %d entries)"
                                   % (operand, len(pool)))
                out += struct.pack('>H', operand)
            elif mnem == 'invokevirtual':
                if operand[0] == 'idx':
                    idx = operand[1]
                    if idx >= len(pool):
                        raise AsmError(line_no,
                                       "invokevirtual index %d out of range (pool has %d entries)"
                                       % (idx, len(pool)))
                    if pool[idx][0] != 'method':
                        raise AsmError(line_no,
                                       "invokevirtual index %d is not a method entry" % idx)
                else:
                    idx = method_index[operand[1]]
                out += struct.pack('>H', idx)
            elif mnem in BRANCHES:
                target = m.labels.get(operand)
                if target is None:
                    raise AsmError(line_no, "branch to undefined label '%s'" % operand)
                code_offset = 0 if is_main else 4
                insn_addr = m.address + code_offset \
                    + (len(out) - method_start - code_offset - 1)
                offset = target - insn_addr
                if not (-0x8000 <= offset <= 0x7FFF):
                    raise AsmError(line_no,
                                   "branch offset %d out of int16 range" % offset)
                out += struct.pack('>h', offset)
            elif mnem == 'wide':
                out.append(OPCODES[operand[0]])
                out += struct.pack('>H', operand[1])
    return bytes(out)


def _emit_constants(pool, methods_by_name):
    out = bytearray()
    for kind, value in pool:
        if kind == 'int':
            out += struct.pack('<I', value & 0xFFFFFFFF)
        else:
            out += struct.pack('<I', methods_by_name[value].address)
    return bytes(out)


def assemble(source):
    """Returns (program_bin, constants_bin). Raises AsmError on any error."""
    methods, methods_by_name, consts = _parse(source)
    program_size = _layout(methods)
    pool, method_index = _build_pool(methods, methods_by_name, consts)
    program = _emit_program(methods, pool, method_index)
    constants = _emit_constants(pool, methods_by_name)
    assert len(program) == program_size
    return program, constants


def main(argv=None):
    parser = argparse.ArgumentParser(
        description='Assembler for the micsim instruction set (.asm -> .bin)')
    parser.add_argument('input', help='.asm source file')
    parser.add_argument('-p', '--program', default='program.bin',
                        help='output program segment file (default: program.bin)')
    parser.add_argument('-c', '--constants', default='constants.bin',
                        help='output constants segment file (default: constants.bin)')
    args = parser.parse_args(argv)

    try:
        with open(args.input, 'r', encoding='utf-8') as f:
            source = f.read()
        program, constants = assemble(source)
    except AsmError as e:
        print('[ERROR] line %d: %s' % (e.line_no, e.msg), file=sys.stderr)
        return 1
    except OSError as e:
        print('[ERROR] %s' % e, file=sys.stderr)
        return 1

    with open(args.program, 'wb') as f:
        f.write(program)
    with open(args.constants, 'wb') as f:
        f.write(constants)

    print('program size: %d bytes (method area included)' % len(program))
    print('constants: %d entries (%d bytes)' % (len(constants) // 4, len(constants)))
    if len(program) > PROGRAM_SEGMENT_DEFAULT:
        print('[WARN] program larger than the default %d-byte segment; '
              'run micsim with -p <size>' % PROGRAM_SEGMENT_DEFAULT, file=sys.stderr)
    if len(constants) > CONSTANT_SEGMENT_DEFAULT:
        print('[WARN] constants larger than the default %d-byte segment; '
              'run micsim with -c <size>' % CONSTANT_SEGMENT_DEFAULT, file=sys.stderr)
    return 0


if __name__ == '__main__':
    sys.exit(main())
