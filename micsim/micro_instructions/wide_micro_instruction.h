#pragma once

#include "../micro_instruction.h"

constexpr uint32_t WIDE_ADDR = 0xCA;

// WIDE is now a single dispatch: the widened opcode is already at the
// queue front (the dispatch that consumed WIDE advanced the queue), so
// JMPC consumes it directly -- the old wait + fetch cycle is gone.
class WIDE2MicroInstruction : public MicroInstruction {
public:
	WIDE2MicroInstruction() {
		inst_.jmpc = 1;
		// dispatch base for wide opcodes: MPC = 0x100 | widened_opcode
		inst_.next_address = 0x100;
	}
};
