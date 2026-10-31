#pragma once
#include "../micro_instruction.h"


constexpr uint32_t BIPUSH_ADDR = 0x10;

class BIPUSH1MicroInstruction : public MicroInstruction {
public:
	BIPUSH1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
	}
};

class BIPUSH2MicroInstruction : public MicroInstruction {
public:
	BIPUSH2MicroInstruction() {
		// The operand is behind this instruction's opcode byte (popped
		// by BIPUSH1's retire): sign-extend it, push it.
		ALU_PASS_A(inst_);
		REG_ASSIGN2_A(inst_, MBR_SIGNED, tos, mdr);
		inst_.mem_wr = 1;
		END_MIR(inst_);
	}
};
