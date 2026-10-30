#pragma once
#include "../micro_instruction.h"


constexpr uint32_t NOP_ADDR = 0x0;

// NOP is now the machine's decode step: it dispatches the next opcode
// directly (goto(MBR)). MAIN no longer exists.
class NOPMicroInstruction : public MicroInstruction {
public:
	NOPMicroInstruction() {
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
