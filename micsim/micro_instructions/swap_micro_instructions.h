#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t SWAP_ADDR = 0x5C;

class SWAP1MicroInstruction : public MicroInstruction {
public:
	SWAP1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN(inst_, SP, mar);
		inst_.mem_rd = 1;
		inst_.next_address = SWAP_ADDR + 1;
	}
};

class SWAP2MicroInstruction : public MicroInstruction {
public:
	SWAP2MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, SP, mar);
		inst_.next_address = SWAP_ADDR + 2;
	}
};

class SWAP3MicroInstruction : public MicroInstruction {
public:
	SWAP3MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, MDR, h);
		inst_.mem_wr = 1;
		inst_.next_address = SWAP_ADDR + 3;
	}
};

class SWAP4MicroInstruction : public MicroInstruction {
public:
	SWAP4MicroInstruction() {
		ALU_ASSIGN_B(inst_);
		REG_ASSIGN(inst_, TOS, mdr);
		inst_.next_address = SWAP_ADDR + 4;
	}
};

class SWAP5MicroInstruction : public MicroInstruction {
public:
	SWAP5MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN(inst_, SP, mar);
		inst_.mem_wr = 1;
		inst_.next_address = SWAP_ADDR + 5;
	}
};

class SWAP6MicroInstruction : public MicroInstruction {
public:
	SWAP6MicroInstruction() {
		ALU_ASSIGN_H(inst_);
		REG_ASSIGN0(inst_, tos);
		inst_.next_address = MAIN_ADDR;
	}
};