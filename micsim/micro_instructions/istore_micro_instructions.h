#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t ISTORE_ADDR = 0x36;

class ISTORE1MicroInstruction : public MicroInstruction {
public:
	ISTORE1MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, LV, h);
		inst_.next_address = ISTORE_ADDR + 1;
	}
};

class ISTORE2MicroInstruction : public MicroInstruction {
public:
	ISTORE2MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, mar);
		inst_.next_address = ISTORE_ADDR + 2;
	}
};

class ISTORE3MicroInstruction : public MicroInstruction {
public:
	ISTORE3MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, TOS, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = ISTORE_ADDR + 3;
	}
};

class ISTORE4MicroInstruction : public MicroInstruction {
public:
	ISTORE4MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = ISTORE_ADDR + 4;
	}
};

class ISTORE5MicroInstruction : public MicroInstruction {
public:
	ISTORE5MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = ISTORE_ADDR + 5;
	}
};

class ISTORE6MicroInstruction : public MicroInstruction {
public:
	ISTORE6MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.next_address = MAIN_ADDR;
	}
};