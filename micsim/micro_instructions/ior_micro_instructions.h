#pragma once
#include "../micro_instruction.h"


constexpr uint32_t IOR_ADDR = 0x81;

class IOR1MicroInstruction : public MicroInstruction {
public:
	IOR1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
	}
};

class IOR2MicroInstruction : public MicroInstruction {
public:
	IOR2MicroInstruction() {
		// read-latency pad: the operand delivers at this cycle's edge
		// and is visible from IOR3.
	}
};

class IOR3MicroInstruction : public MicroInstruction {
public:
	IOR3MicroInstruction() {
		ALU_OR(inst_);
		REG_READ_A(inst_, TOS);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		END_MIR(inst_);
	}
};
