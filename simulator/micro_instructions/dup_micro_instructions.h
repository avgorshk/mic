#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t DUP_ADDR = 0x5A;

class DUP1MicroInstruction : public MicroInstruction {
public:
	DUP1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = DUP_ADDR + 1;
	}
};

class DUP2MicroInstruction : public MicroInstruction {
public:
	DUP2MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, TOS, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = MAIN_ADDR;
	}
};