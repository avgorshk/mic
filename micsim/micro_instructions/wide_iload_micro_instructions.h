#pragma once
#include "../micro_instruction.h"

#include "iload_micro_instructions.h"

constexpr uint32_t WIDE_ILOAD_ADDR = (ILOAD_ADDR | 0x100);

class WIDE_ILOAD1MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD1MicroInstruction() {
		// The 2-byte index is behind the two opcode bytes (WIDE + the
		// inner opcode, popped together by WIDE_ILOAD1's retire):
		// MAR = LV + index in one cycle.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR2_UNSIGNED);
		REG_ASSIGN(inst_, LV, mar);
		inst_.mem_rd = 1;
	}
};

class WIDE_ILOAD2MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
	}
};

class WIDE_ILOAD3MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD3MicroInstruction() {
		inst_.mem_wr = 1;
	}
};

class WIDE_ILOAD4MicroInstruction : public MicroInstruction {
public:
	WIDE_ILOAD4MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		END_MIR(inst_);
	}
};
