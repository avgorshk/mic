#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t ISHR_ADDR = 0x1C;

// ishr: TOS = TOS >> 1 (arithmetic shift right).
// Uses the ALU's sra1 function, which no other micro-routine uses.
class ISHRMicroInstruction : public MicroInstruction {
public:
	ISHRMicroInstruction() {
		ALUFunction alu = { 0 };
		alu.func = ALUFunctionTypes::OR;
		alu.enb = 1;   // B-bus = TOS
		alu.sra1 = 1;  // result >>= 1 with sign fill
		inst_.alu = alu;
		REG_ASSIGN(inst_, TOS, tos);
		inst_.next_address = MAIN_ADDR;
	}
};
