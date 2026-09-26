#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t GOTO_ADDR = 0xA7;

class GOTO1MicroInstruction : public MicroInstruction {
public:
	GOTO1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN(inst_, PC, opc);
		inst_.next_address = GOTO_ADDR + 1;
	}
};

class GOTO2MicroInstruction : public MicroInstruction {
public:
	GOTO2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = GOTO_ADDR + 2;
	}
};

class GOTO3MicroInstruction : public MicroInstruction {
public:
	GOTO3MicroInstruction() {
		ALU_SLL8_B(inst_);
		REG_ASSIGN(inst_, MBR_SIGNED, pc);
		inst_.next_address = GOTO_ADDR + 3;
	}
};

class GOTO4MicroInstruction : public MicroInstruction {
public:
	GOTO4MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN(inst_, MBR_UNSIGNED, h);
		inst_.next_address = GOTO_ADDR + 4;
	}
};

class GOTO5MicroInstruction : public MicroInstruction {
public:
	GOTO5MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, OPC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = GOTO_ADDR + 5;
	}
};

class GOTO6MicroInstruction : public MicroInstruction {
public:
	GOTO6MicroInstruction() {
		inst_.next_address = MAIN_ADDR;
	}
};