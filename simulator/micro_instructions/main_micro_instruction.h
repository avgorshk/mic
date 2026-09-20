#pragma once

#include "../micro_instruction.h"

constexpr uint32_t MAIN_ADDR = 0xFF;

class MAINMicroInstruction : public MicroInstruction {
public:
	MAINMicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::SUM;
		alu.enb = 1;
		alu.inc = 1;

		inst_.alu = alu;
		inst_.read = READ_PC;
		inst_.write_pc = 1;
		inst_.jmpc = 1;
		inst_.mem_fetch = 1;
	}
};