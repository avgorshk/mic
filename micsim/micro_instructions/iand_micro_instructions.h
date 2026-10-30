#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IAND_ADDR = 0x7E;

class IAND1MicroInstruction : public MicroInstruction {
public:
	IAND1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IAND_ADDR + 1;
	}
};

class IAND2MicroInstruction : public MicroInstruction {
public:
	IAND2MicroInstruction() {
		// fill the read-latency slot with MAIN's PC+1 + operand fetch
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = IAND_ADDR + 2;
	}
};

class IAND3MicroInstruction : public MicroInstruction {
public:
	IAND3MicroInstruction() {
		ALU_AND(inst_);
		REG_READ_A(inst_, TOS);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};