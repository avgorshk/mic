#pragma once

#include "../micro_instruction.h"

constexpr uint32_t HALT_ADDR = 0xFF;

class HALTMicroInstruction : public MicroInstruction {
public:
	HALTMicroInstruction() {
		inst_.next_address = HALT_SIGNAL;
	}
};