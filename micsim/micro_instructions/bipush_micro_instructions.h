#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t BIPUSH_ADDR = 0x10;

class BIPUSH1MicroInstruction : public MicroInstruction {
public:
	BIPUSH1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = BIPUSH_ADDR + 1;
	}
};

class BIPUSH2MicroInstruction : public MicroInstruction {
public:
	BIPUSH2MicroInstruction() {
		// The operand is already at the queue front: sign-extend it,
		// push it, and dispatch. The A-bus read consumes the operand,
		// then the dispatch snapshot sees the next opcode.
		ALU_PASS_A(inst_);
		REG_ASSIGN2_A(inst_, MBR_SIGNED, tos, mdr);
		inst_.mem_wr = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
