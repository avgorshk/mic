#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IINC_ADDR = 0x84;

class IINC1MicroInstruction : public MicroInstruction {
public:
	IINC1MicroInstruction() {
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR_UNSIGNED);
		REG_ASSIGN(inst_, LV, mar);
		inst_.mem_rd = 1;
		inst_.next_address = IINC_ADDR + 1;
	}
};

class IINC2MicroInstruction : public MicroInstruction {
public:
	IINC2MicroInstruction() {
		// read-latency pad: the word delivers at this cycle's edge and
		// is visible from IINC3.
		inst_.next_address = IINC_ADDR + 2;
	}
};

class IINC3MicroInstruction : public MicroInstruction {
public:
	IINC3MicroInstruction() {
		// A = the const (consumed at this cycle's edge), B = the word.
		// The dispatch snapshot then sees the next opcode.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR_SIGNED);
		REG_ASSIGN(inst_, MDR, mdr);
		inst_.mem_wr = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
