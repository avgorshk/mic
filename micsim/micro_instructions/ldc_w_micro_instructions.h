#pragma once

#include "iload_micro_instructions.h"

constexpr uint32_t LDC_W_ADDR = 0x13;

class LDC_W1MicroInstruction : public MicroInstruction {
public:
	LDC_W1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = LDC_W_ADDR + 1;
	}
};

class LDC_W2MicroInstruction : public MicroInstruction {
public:
	LDC_W2MicroInstruction() {
		ALU_SLL8_B(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, h);
		inst_.next_address = LDC_W_ADDR + 2;
	}
};

class LDC_W3MicroInstruction : public MicroInstruction {
public:
	LDC_W3MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, h);
		inst_.next_address = LDC_W_ADDR + 3;
	}
};

class LDC_W4MicroInstruction : public MicroInstruction {
public:
	LDC_W4MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, CPP, mar);
		inst_.mem_rd = 1;
		inst_.next_address = ILOAD_ADDR + 2;
	}
};