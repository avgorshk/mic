#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t ILOAD_ADDR = 0x17;

class ILOAD1MicroInstruction : public MicroInstruction {
public:
	ILOAD1MicroInstruction() {
		// The index is already at the queue front; the A-bus read
		// consumes it. MAR = LV + index in one cycle.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR_UNSIGNED);
		REG_ASSIGN(inst_, LV, mar);
		inst_.mem_rd = 1;
		inst_.next_address = ILOAD_ADDR + 1;
	}
};

class ILOAD2MicroInstruction : public MicroInstruction {
public:
	ILOAD2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = ILOAD_ADDR + 2;
	}
};

class ILOAD3MicroInstruction : public MicroInstruction {
public:
	ILOAD3MicroInstruction() {
		inst_.mem_wr = 1;
		inst_.next_address = ILOAD_ADDR + 3;
	}
};

class ILOAD4MicroInstruction : public MicroInstruction {
public:
	ILOAD4MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
