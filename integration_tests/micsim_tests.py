import os
import struct
import subprocess
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'compiler'))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'assembler'))
import micc  # noqa: E402
import asm  # noqa: E402

BIPUSH = 0x10
IADD = 0x62
HALT = 0xFF
LDC_W = 0x13
ISTORE = 0x36

def test_program(micsim):
    program = [BIPUSH, 0x7, BIPUSH, 0x8, IADD, HALT]
    with open("program.bin", "wb") as f:
        f.write(bytes(program))

    cmd = [micsim, "-i", "program.bin", "-o", "output.bin"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 0

    data = []
    with open("output.bin", "rb") as f:
        raw = f.read()
        assert len(raw) % 4 == 0
        count = len(raw) // 4
        data = list(struct.unpack(f"<{count}I", raw))
        assert data[1] == 0xF


def test_constants_flag(micsim):
    # ldc_w 0; istore 0; halt  ->  pushes constants[0] and stores it in
    # main's local 0 (data segment word 0). Exercised via the -k flag.
    program = [LDC_W, 0x0, 0x0, ISTORE, 0x0, HALT]
    with open("ldc_program.bin", "wb") as f:
        f.write(bytes(program))
    with open("ldc_constants.bin", "wb") as f:
        f.write(struct.pack("<I", 42))

    cmd = [micsim, "-i", "ldc_program.bin", "-k", "ldc_constants.bin",
           "-o", "ldc_output.bin"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 0

    with open("ldc_output.bin", "rb") as f:
        raw = f.read()
    data = list(struct.unpack("<%dI" % (len(raw) // 4), raw))
    assert data[0] == 42


def test_max_cycles_flag(micsim):
    # a program that never halts must abort cleanly under --max-cycles
    program = [0x00]  # nop -> main loop forever
    with open("nop_program.bin", "wb") as f:
        f.write(bytes(program))

    cmd = [micsim, "-i", "nop_program.bin", "-o", "nop_output.bin",
           "--max-cycles", "100"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 1
    assert "max cycles reached" in result.stderr


def test_compiler_pipeline(micsim):
    # full pipeline: MICC source -> micc.py -> asm.py -> micsim
    source = '''
int fib(int n) {
    if (n < 2)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int main() {
    int r = fib(8);
    return r;
}
'''
    asm_text = micc.compile_to_asm(source)
    program, constants = asm.assemble(asm_text)
    with open("pipeline_program.bin", "wb") as f:
        f.write(program)
    with open("pipeline_constants.bin", "wb") as f:
        f.write(constants)

    cmd = [micsim, "-i", "pipeline_program.bin", "-k", "pipeline_constants.bin",
           "-o", "pipeline_output.bin"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 0

    with open("pipeline_output.bin", "rb") as f:
        raw = f.read()
    data = list(struct.unpack("<%dI" % (len(raw) // 4), raw))
    assert data[0] == 21


def main():
    if len(sys.argv) < 2:
        print("Usage: micsim_tests.py <micsim_path>")
        return

    micsim = sys.argv[1]
    test_program(micsim)
    test_constants_flag(micsim)
    test_max_cycles_flag(micsim)
    test_compiler_pipeline(micsim)
    print("Passed: 4")

main()