#pragma once
#include "../micro_instruction.h"


constexpr uint32_t SWAP_ADDR = 0x5C;

class SWAP1MicroInstruction : public MicroInstruction {
public:
	SWAP1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN(inst_, SP, mar);
		inst_.mem_rd = 1;
	}
};

class SWAP2MicroInstruction : public MicroInstruction {
public:
	SWAP2MicroInstruction() {
		// read-latency pad: the word delivers at this cycle's edge and
		// is visible from SWAP3.
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, SP, mar);
	}
};

class SWAP3MicroInstruction : public MicroInstruction {
public:
	SWAP3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, h);
		inst_.mem_wr = 1;
	}
};

class SWAP4MicroInstruction : public MicroInstruction {
public:
	SWAP4MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, mdr);
	}
};

class SWAP5MicroInstruction : public MicroInstruction {
public:
	SWAP5MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN(inst_, SP, mar);
		inst_.mem_wr = 1;
	}
};

class SWAP6MicroInstruction : public MicroInstruction {
public:
	SWAP6MicroInstruction() {
		ALU_PASS_A(inst_);
		REG_READ_A(inst_, H);
		REG_ASSIGN0(inst_, tos);
		END_MIR(inst_);
	}
};
