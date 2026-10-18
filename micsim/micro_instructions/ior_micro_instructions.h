#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IOR_ADDR = 0x81;

class IOR1MicroInstruction : public MicroInstruction {
public:
	IOR1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IOR_ADDR + 1;
	}
};

class IOR2MicroInstruction : public MicroInstruction {
public:
	IOR2MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, TOS, h);
		inst_.next_address = IOR_ADDR + 2;
	}
};

class IOR3MicroInstruction : public MicroInstruction {
public:
	IOR3MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = MAIN_ADDR;
	}
};