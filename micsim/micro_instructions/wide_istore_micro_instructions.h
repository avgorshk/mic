#pragma once

#include "istore_micro_instructions.h"

constexpr uint32_t WIDE_ISTORE_ADDR = (ISTORE_ADDR | 0x100);

class WIDE_ISTORE1MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE1MicroInstruction() {
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR2_UNSIGNED);
		REG_ASSIGN(inst_, LV, mar);
		inst_.next_address = WIDE_ISTORE_ADDR + 1;
	}
};

class WIDE_ISTORE2MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE2MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = WIDE_ISTORE_ADDR + 2;
	}
};

class WIDE_ISTORE3MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE3MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = WIDE_ISTORE_ADDR + 3;
	}
};

class WIDE_ISTORE4MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE4MicroInstruction() {
		// pop-read latency pad
		inst_.next_address = WIDE_ISTORE_ADDR + 4;
	}
};

class WIDE_ISTORE5MicroInstruction : public MicroInstruction {
public:
	WIDE_ISTORE5MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
