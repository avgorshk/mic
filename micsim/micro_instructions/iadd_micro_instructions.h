#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IADD_ADDR = 0x62;

class IADD1MicroInstruction : public MicroInstruction {
public:
	IADD1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IADD_ADDR + 1;
	}
};

class IADD2MicroInstruction : public MicroInstruction {
public:
	IADD2MicroInstruction() {
		// fill the read-latency slot with MAIN's PC+1 + operand fetch
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = IADD_ADDR + 2;
	}
};

class IADD3MicroInstruction : public MicroInstruction {
public:
	IADD3MicroInstruction() {
		ALU_ADD(inst_);
		REG_READ_A(inst_, TOS);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
