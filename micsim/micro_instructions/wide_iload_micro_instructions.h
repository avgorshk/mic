#pragma once

#include "iload_micro_instructions.h"

constexpr uint32_t WIDE_ILOAD_ADDR = (ILOAD_ADDR | 0x100);

class WIDE_ILOAD1MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = WIDE_ILOAD_ADDR + 1;
	}
};

class WIDE_ILOAD2MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD2MicroInstruction() {
		ALU_SLL8_B(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, h);
		inst_.next_address = WIDE_ILOAD_ADDR + 2;
	}
};

class WIDE_ILOAD3MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD3MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, h);
		inst_.next_address = WIDE_ILOAD_ADDR + 3;
	}
};

class WIDE_ILOAD4MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD4MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, LV, mar);
		inst_.mem_rd = 1;
		inst_.next_address = ILOAD_ADDR + 2;
	}
};