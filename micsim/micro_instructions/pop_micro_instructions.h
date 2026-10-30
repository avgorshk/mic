#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t POP_ADDR = 0x57;

// POP is fused with the trailing MAIN: the routine runs POP1..POP3 and
// the dispatch happens in POP3 itself, saving one cycle per POP.
//
// Timing: the MAIN that dispatched POP already fetched the next opcode
// (POP has no operands). POP2 absorbs MAIN's work -- PC+1 and the fetch
// of the next instruction's first operand; that fetch delivers at POP3's
// edge, AFTER the dispatch snapshot, so it cannot corrupt the dispatch.
// POP3 moves the popped word into TOS and dispatches directly: JMPC sees
// the pre-edge MBR, which is the next opcode fetched by MAIN.
class POP1MicroInstruction : public MicroInstruction {
public:
	POP1MicroInstruction() {
		ALU_DEC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = POP_ADDR + 1;
	}
};

class POP2MicroInstruction : public MicroInstruction {
public:
	POP2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = POP_ADDR + 2;
	}
};

class POP3MicroInstruction : public MicroInstruction {
public:
	POP3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, tos);
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};