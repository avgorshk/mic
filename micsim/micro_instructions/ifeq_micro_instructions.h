#pragma once
#include "../micro_instruction.h"

#include "f_micro_instructions.h"

constexpr uint32_t IFEQ_ADDR = 0x97;

class IFEQ1MicroInstruction : public MicroInstruction {
public:
	IFEQ1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IFEQ_ADDR + 1;
	}
};

class IFEQ2MicroInstruction : public MicroInstruction {
public:
	IFEQ2MicroInstruction() {
		// read-latency pad
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, opc);
		inst_.next_address = IFEQ_ADDR + 2;
	}
};

class IFEQ3MicroInstruction : public MicroInstruction {
public:
	IFEQ3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.next_address = IFEQ_ADDR + 3;
	}
};

class IFEQ4MicroInstruction : public MicroInstruction {
public:
	IFEQ4MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_READ(inst_, OPC);
		inst_.jamz = 1;
		inst_.next_address = F_ADDR;
	}
};
