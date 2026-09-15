#pragma once

#include "../micro_instruction.h"

constexpr uint32_t ILOAD_ADDR = 0x15;

class ILOAD1MicroInstruction : public MicroInstruction {
public:
	ILOAD1MicroInstruction() {
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
	}
};