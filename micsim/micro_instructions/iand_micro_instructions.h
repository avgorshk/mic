#pragma once
#include "../micro_instruction.h"


constexpr uint32_t IAND_ADDR = 0x7E;

class IAND1MicroInstruction : public MicroInstruction {
public:
	IAND1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
	}
};

class IAND2MicroInstruction : public MicroInstruction {
public:
	IAND2MicroInstruction() {
		// read-latency pad: the operand delivers at this cycle's edge
		// and is visible from IAND3.
	}
};

class IAND3MicroInstruction : public MicroInstruction {
public:
	IAND3MicroInstruction() {
		ALU_AND(inst_);
		REG_READ_A(inst_, TOS);
		REG_ASSIGN2(inst_, MDR, tos, mdr);
		inst_.mem_wr = 1;
		END_MIR(inst_);
	}
};
