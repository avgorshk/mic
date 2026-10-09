#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t POP_ADDR = 0x57;

class POP1MicroInstruction : public MicroInstruction {
public:
	POP1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = POP_ADDR + 1;
	}
};

class POP2MicroInstruction : public MicroInstruction {
public:
	POP2MicroInstruction() {
		inst_.next_address = POP_ADDR + 2;
	}
};

class POP3MicroInstruction : public MicroInstruction {
public:
	POP3MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.next_address = MAIN_ADDR;
	}
};