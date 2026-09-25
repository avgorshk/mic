#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t IF_ICMPEQ_ADDR = 0x9F;

class IF_ICMPEQ1MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IF_ICMPEQ_ADDR + 1;
	}
};

class IF_ICMPEQ2MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ2MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = IF_ICMPEQ_ADDR + 2;
	}
};

class IF_ICMPEQ3MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ3MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, MDR, h);
		inst_.mem_rd = 1;
		inst_.next_address = IF_ICMPEQ_ADDR + 3;
	}
};

class IF_ICMPEQ4MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ4MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, TOS, opc);
		inst_.next_address = IF_ICMPEQ_ADDR + 4;
	}
};

class IF_ICMPEQ5MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ5MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.next_address = IF_ICMPEQ_ADDR + 5;
	}
};

class IF_ICMPEQ6MicroInstruction : public MicroInstruction {
public:
	IF_ICMPEQ6MicroInstruction() {
		ALU_SUB(inst_);
		REG_READ(inst_, OPC);
		inst_.jamz = 1;
		inst_.next_address = MAIN_ADDR;
	}
};