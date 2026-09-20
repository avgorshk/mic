#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t NOP_ADDR = 0x0;

class NOPMicroInstruction : public MicroInstruction {
public:
	NOPMicroInstruction() {
		inst_.next_address = MAIN_ADDR;
	}
};