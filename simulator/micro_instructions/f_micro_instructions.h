#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t F_ADDR = 0xF0;

class F1MicroInstruction : public MicroInstruction {
public:
	F1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.next_address = F_ADDR + 1;
	}
};

class F2MicroInstruction : public MicroInstruction {
public:
	F2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = F_ADDR + 2;
	}
};

class F3MicroInstruction : public MicroInstruction {
public:
	F3MicroInstruction() {
		inst_.next_address = MAIN_ADDR;
	}
};