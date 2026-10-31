#pragma once
#include "../micro_instruction.h"


constexpr uint32_t POP_ADDR = 0x57;

class POP1MicroInstruction : public MicroInstruction {
public:
	POP1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
	}
};

class POP2MicroInstruction : public MicroInstruction {
public:
	POP2MicroInstruction() {
		// read-latency pad: the word delivers at this cycle's edge and
		// is visible from POP3.
	}
};

class POP3MicroInstruction : public MicroInstruction {
public:
	POP3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		END_MIR(inst_);
	}
};
