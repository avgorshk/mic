#pragma once
#include "../micro_instruction.h"


constexpr uint32_t ISTORE_ADDR = 0x36;

class ISTORE1MicroInstruction : public MicroInstruction {
public:
	ISTORE1MicroInstruction() {
		// The index is behind this instruction's opcode byte (popped by
		// ISTORE1's retire): MAR = LV + index in one cycle.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR_UNSIGNED);
		REG_ASSIGN(inst_, LV, mar);
		inst_.mem_rd = 1;
	}
};

class ISTORE2MicroInstruction : public MicroInstruction {
public:
	ISTORE2MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, mdr);
		inst_.mem_wr = 1;
	}
};

class ISTORE3MicroInstruction : public MicroInstruction {
public:
	ISTORE3MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
	}
};

class ISTORE4MicroInstruction : public MicroInstruction {
public:
	ISTORE4MicroInstruction() {
		// pop-read latency pad: the word delivers at this cycle's edge
		// and is visible from ISTORE5.
	}
};

class ISTORE5MicroInstruction : public MicroInstruction {
public:
	ISTORE5MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		END_MIR(inst_);
	}
};
