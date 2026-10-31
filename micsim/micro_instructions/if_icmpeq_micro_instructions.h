#pragma once
#include "../micro_instruction.h"


constexpr uint32_t IF_ICMPEQ_ADDR = 0x9F;

class IF_ICMPEQ1MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
	}
};

class IF_ICMPEQ2MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ2MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
	}
};

class IF_ICMPEQ3MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, h);
		inst_.mem_rd = 1;
	}
};

class IF_ICMPEQ4MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ4MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, opc);
	}
};

class IF_ICMPEQ5MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ5MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
	}
};

class IF_ICMPEQ6MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ6MicroInstruction() {
		// C = H - OPC, flags = C. The dedicated branch MIR that follows
		// evaluates its JAM bits against these flags.
		ALU_SUB(inst_);
		REG_READ_A(inst_, H);
		REG_READ_B(inst_, OPC);
	}
};
