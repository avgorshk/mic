#pragma once
#include "../micro_instruction.h"

#include "goto_micro_instructions.h"

// The taken block, shared by every conditional branch and hardwired at
// TAKEN_BLOCK_INDEX: T computes OPC = the branch instruction's own
// address (the IFU's PC is at the first offset byte, so PC - 1), then
// the shared GOTO2 consumes the offset and writes the PC, and GOTO3 is
// the scheduled refill wait.
class TMicroInstruction : public MicroInstruction {
public:
	TMicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN(inst_, PC, opc);
	}
};
