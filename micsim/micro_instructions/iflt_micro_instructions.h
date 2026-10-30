#pragma once

#include "f_micro_instructions.h"

constexpr uint32_t IFLT_ADDR = 0x9B;

class IFLT1MicroInstruction : public MicroInstruction {
public:
	IFLT1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IFLT_ADDR + 1;
	}
};

class IFLT2MicroInstruction : public MicroInstruction {
public:
	IFLT2MicroInstruction() {
		// read-latency pad
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, opc);
		inst_.next_address = IFLT_ADDR + 2;
	}
};

class IFLT3MicroInstruction : public MicroInstruction {
public:
	IFLT3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.next_address = IFLT_ADDR + 3;
	}
};

class IFLT4MicroInstruction : public MicroInstruction {
public:
	IFLT4MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_READ(inst_, OPC);
		inst_.jamn = 1;
		inst_.next_address = F_ADDR;
	}
};
