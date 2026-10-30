#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t F_ADDR = 0xF0;

class F1MicroInstruction : public MicroInstruction {
public:
	F1MicroInstruction() {
		// Not-taken branch: discard the 2-byte offset. The MBR2 read
		// consumes it; the next opcode is already behind it.
		REG_READ_A(inst_, MBR2_UNSIGNED);
		inst_.next_address = F_ADDR + 1;
	}
};

class F2MicroInstruction : public MicroInstruction {
public:
	F2MicroInstruction() {
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
