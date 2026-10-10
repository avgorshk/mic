#pragma once

#include "../micro_instruction.h"

constexpr uint32_t WIDE_ADDR = 0xC4;

class WIDEMicroInstruction : public MicroInstruction {
public:
	WIDEMicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0x100;
	}
};