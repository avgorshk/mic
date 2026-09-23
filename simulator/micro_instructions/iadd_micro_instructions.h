#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IADD_ADDR = 0x60;

class IADD1MicroInstruction : public MicroInstruction {
public:
	IADD1MicroInstruction() {
		ALU_DEC_B(inst_);

		inst_.read = READ_SP;
		inst_.write_sp = 1;
		inst_.write_mar = 1;

		inst_.mem_rd = 1;

		inst_.next_address = IADD_ADDR + 1;
	}
};

class IADD2MicroInstruction : public MicroInstruction {
public:
	IADD2MicroInstruction() {
		ALU_ASSIGN_B(inst_);

		inst_.read = READ_TOS;
		inst_.write_h = 1;

		inst_.next_address = IADD_ADDR + 2;
	}
};

class IADD3MicroInstruction : public MicroInstruction {
public:
	IADD3MicroInstruction() {
		ALU_ADD(inst_);

		inst_.read = READ_MDR;
		inst_.write_tos = 1;
		inst_.write_mdr = 1;

		inst_.mem_wr = 1;

		inst_.next_address = MAIN_ADDR;
	}
};
