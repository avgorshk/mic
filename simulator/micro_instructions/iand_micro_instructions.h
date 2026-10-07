#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IAND_ADDR = 0x7E;

class IAND1MicroInstruction : public MicroInstruction {
public:
	IAND1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IAND_ADDR + 1;
	}
};

class IAND2MicroInstruction : public MicroInstruction {
public:
	IAND2MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, TOS, h);
		inst_.next_address = IAND_ADDR + 2;
	}
};

class IAND3MicroInstruction : public MicroInstruction {
public:
	IAND3MicroInstruction() {
		ALU_AND(inst_);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = MAIN_ADDR;
	}
};