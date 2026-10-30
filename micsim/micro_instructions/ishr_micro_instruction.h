#pragma once
#include "../micro_instruction.h"


constexpr uint32_t ISHR_ADDR = 0x1C;

// ishr: TOS = TOS >> 1 (arithmetic shift right).
class ISHRMicroInstruction : public MicroInstruction {
public:
	ISHRMicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::OR;
		alu.enb = 1;   // B-bus = TOS
		alu.sra1 = 1;  // result >>= 1 with sign fill
		inst_.alu = alu;
		REG_ASSIGN(inst_, TOS, tos);
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
