"""Unit tests for the micsim assembler (assembler/asm.py).

Run: python compiler_tests/test_asm.py
Exit code 0 = all passed, 1 = at least one failure.
No external dependencies.
"""

import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'assembler'))
import asm  # noqa: E402

RESULTS = []


def check(name, fn):
    try:
        fn()
        RESULTS.append((name, True, ''))
    except Exception as e:
        RESULTS.append((name, False, '%s: %s' % (type(e).__name__, e)))


def expect_error(source, needle):
    try:
        asm.assemble(source)
    except asm.AsmError as e:
        assert needle in e.msg, "error '%s' does not contain '%s'" % (e.msg, needle)
        return
    raise AssertionError('expected AsmError containing %r, got no error' % needle)


ADD2_SRC = '''
; add2: main calls add2(40, 2), stores result in local 0, halts
.method main
.locals 1
bipush 0                 ; OBJREF placeholder
bipush 40                ; arg1
bipush 2                 ; arg2
invokevirtual add2
istore 0
halt

.method add2
.params 3
.locals 0
iload 1
iload 2
iadd
ireturn
'''


def test_add2_bytes():
    """Full add2 program: exact program.bin and constants.bin bytes."""
    prog, consts = asm.assemble(ADD2_SRC)
    expected_prog = bytes([
        0x10, 0x00,             # prologue: bipush 0 (auto, .locals 1)
        0x10, 0x00,             # objref
        0x10, 0x28,             # bipush 40
        0x10, 0x02,             # bipush 2
        0xB4, 0x00, 0x00,       # invokevirtual 0
        0x36, 0x00,             # istore 0
        0xFF,                   # halt
        0x00, 0x03,             # add2 header: params = 3
        0x00, 0x00,             # locals = 0
        0x17, 0x01,             # iload 1
        0x17, 0x02,             # iload 2
        0x62,                   # iadd
        0xAC,                   # ireturn
    ])
    assert prog == expected_prog, 'program bytes differ: %s' % prog.hex()
    expected_consts = struct.pack('<I', 14)   # add2 header at byte 14
    assert consts == expected_consts, 'constants differ: %s' % consts.hex()


def test_add2_asm_file_matches():
    """The committed programs/add2.asm assembles to the same bytes."""
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '..', 'programs', 'add2.asm')
    with open(path, 'r', encoding='utf-8') as f:
        prog, consts = asm.assemble(f.read())
    ref_prog, ref_consts = asm.assemble(ADD2_SRC)
    assert prog == ref_prog, 'file program differs from reference'
    assert consts == ref_consts, 'file constants differ from reference'


def test_main_prologue():
    """main .locals k emits bipush 0 + (k-1) x dup before the code."""
    prog, _ = asm.assemble('''
.method main
.locals 3
halt
''')
    assert prog == bytes([0x10, 0x00, 0x5A, 0x5A, 0xFF]), prog.hex()
    # .locals 0 -> no prologue
    prog, _ = asm.assemble('''
.method main
halt
''')
    assert prog == bytes([0xFF]), prog.hex()


def test_main_not_first():
    expect_error('''
.method foo
halt
''', "first method must be 'main'")


def test_duplicate_method():
    expect_error('''
.method main
halt
.method main
halt
''', "duplicate method")


def test_main_params_forbidden():
    expect_error('''
.method main
.params 2
halt
''', 'main cannot have .params')


def test_bipush_range():
    expect_error('''
.method main
bipush 128
halt
''', 'out of range')
    expect_error('''
.method main
bipush -129
halt
''', 'out of range')
    prog, _ = asm.assemble('''
.method main
bipush -1
bipush 127
halt
''')
    assert prog[:5] == bytes([0x10, 0xFF, 0x10, 0x7F, 0xFF]), prog[:5].hex()


def test_iinc_encoding():
    prog, _ = asm.assemble('''
.method main
iinc 1, -1
iinc 2, 5
halt
''')
    assert prog[:7] == bytes([0x84, 0x01, 0xFF, 0x84, 0x02, 0x05, 0xFF]), prog[:7].hex()
    expect_error('''
.method main
iinc 1, 128
halt
''', 'out of range')


def test_wide_encoding():
    prog, _ = asm.assemble('''
.method main
wide iload 300
wide istore 65535
halt
''')
    assert prog == bytes([0xCA, 0x17, 0x01, 0x2C,
                          0xCA, 0x36, 0xFF, 0xFF,
                          0xFF]), prog.hex()
    expect_error('''
.method main
wide iadd 1
halt
''', "wide expects 'iload' or 'istore'")


def test_branch_backward():
    """goto to a label before the branch: negative offset."""
    prog, _ = asm.assemble('''
.method main
loop:
nop
goto loop
halt
''')
    # loop at 0, nop at 0, goto at 1..3, offset = 0 - 1 = -1
    assert prog == bytes([0x00, 0xA6, 0xFF, 0xFF, 0xFF]), prog.hex()


def test_branch_forward():
    prog, _ = asm.assemble('''
.method main
ifeq L
bipush 1
halt
L:
bipush 0
halt
''')
    # ifeq at 0..2, bipush 1 at 3..4, halt at 5, L at 6, offset = 6 - 0 = 6
    assert prog[:3] == bytes([0x97, 0x00, 0x06]), prog[:3].hex()


def test_branch_undefined_label():
    expect_error('''
.method main
goto nope
halt
''', "undefined label 'nope'")


def test_branch_cross_method():
    expect_error('''
.method main
goto L
halt
.method foo
L:
halt
''', "undefined label 'L'")


def test_duplicate_label():
    expect_error('''
.method main
L:
nop
L:
halt
''', "duplicate label 'L'")


def test_branch_offset_out_of_range():
    nops = 'nop\n' * 33000
    expect_error('''
.method main
goto L
%sL:
halt
''' % nops, 'out of int16 range')


def test_ldc_w_index_range():
    expect_error('''
.method main
ldc_w 0
halt
''', 'out of range (pool has 0 entries)')
    prog, consts = asm.assemble('''
.method main
.const 42
ldc_w 0
halt
''')
    assert prog[:4] == bytes([0x13, 0x00, 0x00, 0xFF]), prog[:4].hex()
    assert consts == struct.pack('<I', 42)


def test_const_negative():
    _, consts = asm.assemble('''
.method main
.const -5
.const 0xDEADBEEF
halt
''')
    assert consts == struct.pack('<II', 0xFFFFFFFB, 0xDEADBEEF), consts.hex()


def test_invokevirtual_name_creates_entry():
    prog, consts = asm.assemble('''
.method main
bipush 0
invokevirtual foo
halt
.method foo
.params 1
halt
''')
    # pool: entry 0 = foo header address. main: prologue none; bipush 0 (2B);
    # invokevirtual idx 0 (3B); halt (1B) = 6 bytes; foo header at 6.
    assert prog == bytes([0x10, 0x00, 0xB4, 0x00, 0x00, 0xFF,
                          0x00, 0x01, 0x00, 0x00, 0xFF]), prog.hex()
    assert consts == struct.pack('<I', 6), consts.hex()


def test_invokevirtual_reuses_const_entry():
    _, consts = asm.assemble('''
.method main
.const foo
bipush 0
invokevirtual foo
halt
.method foo
.params 1
halt
''')
    assert len(consts) == 4, 'expected exactly one pool entry'


def test_invokevirtual_idx_not_method():
    expect_error('''
.method main
.const 42
invokevirtual 0
halt
''', 'is not a method entry')


def test_const_unknown_method():
    expect_error('''
.method main
.const ghost
halt
''', "unknown method 'ghost'")


def test_params_default():
    _, _ = asm.assemble('''
.method main
bipush 0
invokevirtual foo
halt
.method foo
iload 1
ireturn
''')
    # .params omitted -> defaults to 1 (objref only); header = 00 01
    prog, _ = asm.assemble('''
.method main
bipush 0
invokevirtual foo
halt
.method foo
iload 1
ireturn
''')
    assert prog[6:10] == bytes([0x00, 0x01, 0x00, 0x00]), prog[6:10].hex()


def test_ishr_encoding():
    prog, _ = asm.assemble('''
.method main
bipush 12
ishr
halt
''')
    # prologue: none (.locals 0); bipush 12, ishr, halt
    assert prog == bytes([0x10, 0x0C, 0x1C, 0xFF]), prog.hex()


def test_stack_depth_warning():
    src = '.method main\nbipush 0\nbipush 1\ninvokevirtual f0\nhalt\n'
    for i in range(70):
        src += '.method f%d\n.params 2\n' % i
        if i < 69:
            src += 'bipush 0\nbipush 1\ninvokevirtual f%d\n' % (i + 1)
        src += 'ireturn\n'
    warnings = asm.analyze_stack(src)
    assert any('exceeds the default 256-word' in w for w in warnings), \
        'expected stack depth warning, got: %s' % warnings


def test_stack_depth_recursion_note():
    src = '''
.method main
bipush 0
bipush 10
invokevirtual fib
halt

.method fib
.params 2
.locals 0
iload 1
bipush 2
isub
iflt L0
goto L1
L0:
iload 1
ireturn
L1:
bipush 0
iload 1
bipush 1
isub
invokevirtual fib
ireturn
'''
    warnings = asm.analyze_stack(src)
    assert any("recursive method 'fib'" in w for w in warnings), \
        'expected recursion note, got: %s' % warnings
    assert not any('exceeds' in w for w in warnings), warnings


def test_stack_depth_no_warning_for_small_program():
    warnings = asm.analyze_stack('''
.method main
bipush 3
bipush 4
iadd
halt
''')
    assert warnings == [], warnings


def test_unknown_instruction():
    expect_error('''
.method main
frobnicate
halt
''', "unknown instruction 'frobnicate'")


def test_instruction_outside_method():
    expect_error('''
iload 0
''', 'instruction outside a method')


def test_empty_source():
    expect_error('', 'no .method defined')


check('add2 exact bytes', test_add2_bytes)
check('add2.asm file matches reference', test_add2_asm_file_matches)
check('main prologue auto-emission', test_main_prologue)
check('main must be first', test_main_not_first)
check('duplicate method rejected', test_duplicate_method)
check('main .params forbidden', test_main_params_forbidden)
check('bipush range check', test_bipush_range)
check('iinc encoding', test_iinc_encoding)
check('wide encoding', test_wide_encoding)
check('backward branch offset', test_branch_backward)
check('forward branch offset', test_branch_forward)
check('undefined label rejected', test_branch_undefined_label)
check('cross-method branch rejected', test_branch_cross_method)
check('duplicate label rejected', test_duplicate_label)
check('branch offset out of int16', test_branch_offset_out_of_range)
check('ldc_w index range', test_ldc_w_index_range)
check('negative and hex constants', test_const_negative)
check('invokevirtual by name creates entry', test_invokevirtual_name_creates_entry)
check('invokevirtual reuses .const entry', test_invokevirtual_reuses_const_entry)
check('invokevirtual index to non-method rejected', test_invokevirtual_idx_not_method)
check('ishr encoding', test_ishr_encoding)
check('stack depth warning (deep call chain)', test_stack_depth_warning)
check('stack depth recursion note', test_stack_depth_recursion_note)
check('stack depth silent for small programs', test_stack_depth_no_warning_for_small_program)
check('.const unknown method rejected', test_const_unknown_method)
check('.params defaults to 1', test_params_default)
check('unknown instruction rejected', test_unknown_instruction)
check('instruction outside method rejected', test_instruction_outside_method)
check('empty source rejected', test_empty_source)


def main():
    failed = 0
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
