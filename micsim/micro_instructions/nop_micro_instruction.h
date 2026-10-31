#pragma once
#include "../micro_instruction.h"


constexpr uint32_t NOP_ADDR = 0x0;

// nop: a no-op MIR. The queue's copy terminates on the end bit; the
// opcode itself is popped by this routine's first MIR at its retire.
class NOPMicroInstruction : public MicroInstruction {
public:
	NOPMicroInstruction() {
		END_MIR(inst_);
	}
};
