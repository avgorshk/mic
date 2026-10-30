#pragma once

#include "iload_micro_instructions.h"

constexpr uint32_t WIDE_ILOAD_ADDR = (ILOAD_ADDR | 0x100);

class WIDE_ILOAD1MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD1MicroInstruction() {
		// The 2-byte index is at the queue front; the MBR2 read
		// consumes it. MAR = LV + index in one cycle.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR2_UNSIGNED);
		REG_ASSIGN(inst_, LV, mar);
		inst_.mem_rd = 1;
		inst_.next_address = WIDE_ILOAD_ADDR + 1;
	}
};

class WIDE_ILOAD2MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = WIDE_ILOAD_ADDR + 2;
	}
};

class WIDE_ILOAD3MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD3MicroInstruction() {
		inst_.mem_wr = 1;
		inst_.next_address = WIDE_ILOAD_ADDR + 3;
	}
};

class WIDE_ILOAD4MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD4MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
