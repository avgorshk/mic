import struct
import subprocess
import sys

BIPUSH = 0x10
IADD = 0x62
HALT = 0xFF

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

def main():
    if len(sys.argv) < 2:
        print("Usage: micsim_tests.py <micsim_path>")
        return

    micsim = sys.argv[1]
    test_program(micsim)
    print("Passed: 1")

main()