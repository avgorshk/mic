#pragma once

#include "../micro_instruction.h"

constexpr uint32_t MAIN_ADDR = 0xFF;

class MAINMicroInstruction : public MicroInstruction {
public:
	MAINMicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.jmpc = 1;
		inst_.mem_fetch = 1;
	}
};