#pragma once
#include "../micro_instruction.h"


constexpr uint32_t IFLT_ADDR = 0x9B;

class IFLT1MicroInstruction : public MicroInstruction {
public:
	IFLT1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
	}
};

class IFLT2MicroInstruction : public MicroInstruction {
public:
	IFLT2MicroInstruction() {
		// read-latency pad
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, opc);
	}
};

class IFLT3MicroInstruction : public MicroInstruction {
public:
	IFLT3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
	}
};

class IFLT4MicroInstruction : public MicroInstruction {
public:
	IFLT4MicroInstruction() {
		// C = OPC, flags = OPC. The dedicated branch MIR that follows
		// evaluates its JAM bits against these flags.
		ALU_PASS_B(inst_);
		REG_READ_B(inst_, OPC);
	}
};
