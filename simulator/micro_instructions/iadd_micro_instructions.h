#pragma once

#include "../micro_instruction.h"

constexpr uint32_t IADD_ADDR = 0x60;

class IADD1MicroInstruction : public MicroInstruction {
public:
	IADD1MicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::SUM;
		alu.ena = 0;
		alu.enb = 1;
		alu.inva = 1;
		alu.inc = 0;
		inst_.alu = alu;

		inst_.read = READ_SP;
		inst_.write_sp = 1;
		inst_.write_mar = 1;

		inst_.mem_rd = 1;

		inst_.next_address = IADD_ADDR + 1;
	}
};