#pragma once

#include "../micro_instruction.h"

constexpr uint32_t MAIN_ADDR = 0x100;

// MAIN is now a pure dispatch: the IFU's queue front holds the opcode
// (its predecessor fetched it in the background), JMPC consumes it and
// the queue advances -- the routine's first cycle sees the first
// operand with no latency and no bookkeeping.
class MAINMicroInstruction : public MicroInstruction {
public:
	MAINMicroInstruction() {
		inst_.jmpc = 1;
	}
};
