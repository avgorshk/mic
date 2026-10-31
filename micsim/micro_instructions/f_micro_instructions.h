#pragma once
#include "../micro_instruction.h"

// The not-taken continuation of a split conditional branch: consumes the
// 2-byte offset. It sits directly after the branch MIR of each branch
// routine, so the queue resumes copying exactly here on not-taken.
// (The old F2 dispatch step is gone: the next instruction is decoded by
// the decoder, not dispatched.)
class F1MicroInstruction : public MicroInstruction {
public:
	F1MicroInstruction() {
		// Not-taken branch: discard the 2-byte offset. The MBR2 read
		// consumes it at the retire.
		REG_READ_A(inst_, MBR2_UNSIGNED);
		END_MIR(inst_);
	}
};
