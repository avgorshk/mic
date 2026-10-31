#pragma once
#include "../micro_instruction.h"


constexpr uint32_t IFEQ_ADDR = 0x97;

class IFEQ1MicroInstruction : public MicroInstruction {
public:
	IFEQ1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
	}
};

class IFEQ2MicroInstruction : public MicroInstruction {
public:
	IFEQ2MicroInstruction() {
		// read-latency pad
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, opc);
	}
};

class IFEQ3MicroInstruction : public MicroInstruction {
public:
	IFEQ3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
	}
};

class IFEQ4MicroInstruction : public MicroInstruction {
public:
	IFEQ4MicroInstruction() {
		// C = OPC, flags = OPC. The dedicated branch MIR that follows
		// evaluates its JAM bits against these flags.
		ALU_PASS_B(inst_);
		REG_READ_B(inst_, OPC);
	}
};
