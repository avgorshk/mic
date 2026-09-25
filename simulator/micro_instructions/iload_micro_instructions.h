#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t ILOAD_ADDR = 0x15;

class ILOAD1MicroInstruction : public MicroInstruction {
public:
	ILOAD1MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, LV, h);
		inst_.next_address = ILOAD_ADDR + 1;
	}
};

class ILOAD2MicroInstruction : public MicroInstruction {
public:
	ILOAD2MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, mar);
		inst_.mem_rd = 1;
		inst_.next_address = ILOAD_ADDR + 2;
	}
};

class ILOAD3MicroInstruction : public MicroInstruction {
public:
	ILOAD3MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = ILOAD_ADDR + 3;
	}
};

class ILOAD4MicroInstruction : public MicroInstruction {
public:
	ILOAD4MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.mem_wr = 1;
		inst_.next_address = ILOAD_ADDR + 4;
	}
};

class ILOAD5MicroInstruction : public MicroInstruction {
public:
	ILOAD5MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.next_address = MAIN_ADDR;
	}
};