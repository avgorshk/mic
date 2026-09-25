#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t BIPUSH_ADDR = 0x10;

class BIPUSH1MicroInstruction : public MicroInstruction {
public:
	BIPUSH1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = BIPUSH_ADDR + 1;
	}
};

class BIPUSH2MicroInstruction : public MicroInstruction {
public:
	BIPUSH2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.next_address = BIPUSH_ADDR + 2;
	}
};

class BIPUSH3MicroInstruction : public MicroInstruction {
public:
	BIPUSH3MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN2(inst_, MBR_SIGNED, tos, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = MAIN_ADDR;
	}
};