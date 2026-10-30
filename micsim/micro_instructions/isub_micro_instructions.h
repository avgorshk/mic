#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t ISUB_ADDR = 0x65;

class ISUB1MicroInstruction : public MicroInstruction {
public:
	ISUB1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = ISUB_ADDR + 1;
	}
};

class ISUB2MicroInstruction : public MicroInstruction {
public:
	ISUB2MicroInstruction() {
		// read-latency pad: the operand delivers at this cycle's edge
		// and is visible from ISUB3.
		inst_.next_address = ISUB_ADDR + 2;
	}
};

class ISUB3MicroInstruction : public MicroInstruction {
public:
	ISUB3MicroInstruction() {
		ALU_SUB(inst_);
		REG_READ_A(inst_, TOS);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
