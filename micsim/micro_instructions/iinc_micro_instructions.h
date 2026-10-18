#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IINC_ADDR = 0x84;

class IINC1MicroInstruction : public MicroInstruction {
public:
	IINC1MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, LV, h);
		inst_.next_address = IINC_ADDR + 1;
	}
};

class IINC2MicroInstruction : public MicroInstruction {
public:
	IINC2MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, mar);
		inst_.mem_rd = 1;
		inst_.next_address = IINC_ADDR + 2;
	}
};

class IINC3MicroInstruction : public MicroInstruction {
public:
	IINC3MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = IINC_ADDR + 3;
	}
};

class IINC4MicroInstruction : public MicroInstruction {
public:
	IINC4MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, MDR, h);
		inst_.next_address = IINC_ADDR + 4;
	}
};

class IINC5MicroInstruction : public MicroInstruction {
public:
	IINC5MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = IINC_ADDR + 5;
	}
};

class IINC6MicroInstruction : public MicroInstruction {
public:
	IINC6MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, MBR_SIGNED, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = MAIN_ADDR;
	}
};