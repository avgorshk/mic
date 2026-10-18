#pragma once

#include "istore_micro_instructions.h"

constexpr uint32_t WIDE_ISTORE_ADDR = (ISTORE_ADDR | 0x100);

class WIDE_ISTORE1MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = WIDE_ISTORE_ADDR + 1;
	}
};

class WIDE_ISTORE2MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE2MicroInstruction() {
		ALU_SLL8_B(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, h);
		inst_.next_address = WIDE_ISTORE_ADDR + 2;
	}
};

class WIDE_ISTORE3MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE3MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, h);
		inst_.next_address = WIDE_ISTORE_ADDR + 3;
	}
};

class WIDE_ISTORE4MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE4MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, LV, mar);
		inst_.mem_rd = 1;
		inst_.next_address = ISTORE_ADDR + 2;
	}
};