#pragma once

#include "goto_micro_instructions.h"

constexpr uint32_t T_ADDR = 0x1F0;

class TMicroInstruction : public MicroInstruction {
public:
	TMicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN(inst_, PC, opc);
		inst_.next_address = GOTO_ADDR + 1;
	}
};