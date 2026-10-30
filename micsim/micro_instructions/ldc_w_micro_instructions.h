#pragma once
#include "../micro_instruction.h"

#include "iload_micro_instructions.h"

constexpr uint32_t LDC_W_ADDR = 0x13;

class LDC_W1MicroInstruction : public MicroInstruction {
public:
	LDC_W1MicroInstruction() {
		// The 2-byte pool index is already at the queue front; the
		// MBR2 read consumes both bytes. MAR = CPP + index in one cycle.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR2_UNSIGNED);
		REG_ASSIGN(inst_, CPP, mar);
		inst_.mem_rd = 1;
		inst_.next_address = LDC_W_ADDR + 1;
	}
};

class LDC_W2MicroInstruction : public MicroInstruction {
public:
	LDC_W2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = LDC_W_ADDR + 2;
	}
};

class LDC_W3MicroInstruction : public MicroInstruction {
public:
	LDC_W3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
