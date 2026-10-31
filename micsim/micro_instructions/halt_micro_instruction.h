#pragma once

#include "../micro_instruction.h"

constexpr uint32_t HALT_ADDR = 0xFF;

class HALTMicroInstruction : public MicroInstruction {
public:
	HALTMicroInstruction() {
		HALT_MIR(inst_);
		END_MIR(inst_);
	}
};
