#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t ILOAD_ADDR = 0x15;

class ILOAD1MicroInstruction : public MicroInstruction {
public:
	ILOAD1MicroInstruction() {
		ALU_ASSIGN_B(inst_);

		inst_.read = READ_LV;
		inst_.write_h = 1;

		inst_.next_address = ILOAD_ADDR + 1;
	}
};

class ILOAD2MicroInstruction : public MicroInstruction {
public:
	ILOAD2MicroInstruction() {
		ALU_ADD(inst_);

		inst_.read = READ_MBR_UNSIGNED;
		inst_.write_mar = 1;
		inst_.mem_rd = 1;

		inst_.next_address = ILOAD_ADDR + 2;
	}
};

class ILOAD3MicroInstruction : public MicroInstruction {
public:
	ILOAD3MicroInstruction() {
		ALU_INC_B(inst_);

		inst_.read = READ_SP;
		inst_.write_sp = 1;
		inst_.write_mar = 1;

		inst_.next_address = ILOAD_ADDR + 3;
	}
};

class ILOAD4MicroInstruction : public MicroInstruction {
public:
	ILOAD4MicroInstruction() {
		ALU_INC_B(inst_);

		inst_.read = READ_PC;
		inst_.write_pc = 1;

		inst_.mem_fetch = 1;
		inst_.mem_wr = 1;

		inst_.next_address = ILOAD_ADDR + 4;
	}
};

class ILOAD5MicroInstruction : public MicroInstruction {
public:
	ILOAD5MicroInstruction() {
		ALU_ASSIGN_B(inst_);

		inst_.read = READ_MDR;
		inst_.write_tos = 1;

		inst_.next_address = MAIN_ADDR;
	}
};