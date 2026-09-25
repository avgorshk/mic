#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IADD_ADDR = 0x60;

class IADD1MicroInstruction : public MicroInstruction {
public:
	IADD1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IADD_ADDR + 1;
	}
};

class IADD2MicroInstruction : public MicroInstruction {
public:
	IADD2MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, TOS, h);
		inst_.next_address = IADD_ADDR + 2;
	}
};

class IADD3MicroInstruction : public MicroInstruction {
public:
	IADD3MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = MAIN_ADDR;
	}
};
