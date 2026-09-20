#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t ILOAD_ADDR = 0x15;

class ILOAD1MicroInstruction : public MicroInstruction {
public:
	ILOAD1MicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::OR;
		alu.enb = 1;

		inst_.alu = alu;
		inst_.read = READ_LV;
		inst_.write_h = 1;

		inst_.next_address = ILOAD_ADDR + 1;
	}
};

class ILOAD2MicroInstruction : public MicroInstruction {
public:
	ILOAD2MicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::SUM;
		alu.ena = 1;
		alu.enb = 1;

		inst_.alu = alu;
		inst_.read = READ_MBR_UNSIGNED;
		inst_.write_mar = 1;
		inst_.mem_rd = 1;

		inst_.next_address = ILOAD_ADDR + 2;
	}
};

class ILOAD3MicroInstruction : public MicroInstruction {
public:
	ILOAD3MicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::SUM;
		alu.enb = 1;
		alu.inc = 1;

		inst_.alu = alu;
		inst_.read = READ_SP;
		inst_.write_sp = 1;
		inst_.write_mar = 1;

		inst_.next_address = ILOAD_ADDR + 3;
	}
};

class ILOAD4MicroInstruction : public MicroInstruction {
public:
	ILOAD4MicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::SUM;
		alu.enb = 1;
		alu.inc = 1;

		inst_.alu = alu;
		inst_.read = READ_PC;
		inst_.write_pc = 1;

		inst_.mem_fetch = 1;
		inst_.mem_wr = 1;

		inst_.next_address = ILOAD_ADDR + 4;
	}
};

class ILOAD5MicroInstruction : public MicroInstruction {
public:
	ILOAD5MicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::OR;
		alu.enb = 1;

		inst_.alu = alu;
		inst_.read = READ_MDR;
		inst_.write_tos = 1;

		inst_.next_address = MAIN_ADDR;
	}
};