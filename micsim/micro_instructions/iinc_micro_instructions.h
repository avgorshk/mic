#pragma once
#include "../micro_instruction.h"


constexpr uint32_t IINC_ADDR = 0x84;

class IINC1MicroInstruction : public MicroInstruction {
public:
	IINC1MicroInstruction() {
		// The index is behind this instruction's opcode byte (popped by
		// IINC1's retire): MAR = LV + index, read the word.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR_UNSIGNED);
		REG_ASSIGN(inst_, LV, mar);
		inst_.mem_rd = 1;
	}
};

class IINC2MicroInstruction : public MicroInstruction {
public:
	IINC2MicroInstruction() {
		// read-latency pad: the word delivers at this cycle's edge and
		// is visible from IINC3.
	}
};

class IINC3MicroInstruction : public MicroInstruction {
public:
	IINC3MicroInstruction() {
		// A = the const, B = the word: add and store back.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR_SIGNED);
		REG_ASSIGN(inst_, MDR, mdr);
		inst_.mem_wr = 1;
		END_MIR(inst_);
	}
};
