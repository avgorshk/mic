#pragma once
#include "../micro_instruction.h"

// The dedicated branch MIR of a split conditional branch: no datapath
// work. The queue stops copying when it meets the jump bit and waits
// for this MIR's retirement, where the JAM flags are evaluated against
// the predecessor's N/Z flags carried in the MemRead stage.
class BranchZMicroInstruction : public MicroInstruction {
public:
	BranchZMicroInstruction() {
		JUMP_MIR(inst_);
		inst_.jamz = 1;
	}
};

class BranchNMicroInstruction : public MicroInstruction {
public:
	BranchNMicroInstruction() {
		JUMP_MIR(inst_);
		inst_.jamn = 1;
	}
};
