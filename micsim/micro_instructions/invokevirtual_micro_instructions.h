#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t INVOKEVIRTUAL_ADDR = 0xB4;

// 16 cycles (was 22). The index comes straight from MBR2 (no 3-cycle
// assembly), the jump to the callee is write_pc (flush + refill), and
// the four header bytes are consumed directly from the refilled queue:
// each MBR read advances the queue, so the fetch cycles and their waits
// collapse away. IV5 stalls once while the first callee block is in
// flight -- the honest cost of the jump.
class INVOKEVIRTUAL1MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL1MicroInstruction() {
		ALU_PASS_A(inst_);
		REG_ASSIGN_A(inst_, MBR2_UNSIGNED, h);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 1;
	}
};

class INVOKEVIRTUAL2MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL2MicroInstruction() {
		ALU_ADD(inst_);
		REG_READ(inst_, H);
		REG_ASSIGN(inst_, CPP, mar);
		inst_.mem_rd = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 2;
	}
};

class INVOKEVIRTUAL3MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL3MicroInstruction() {
		// The IFU's PC is already past the 2-byte index: the return
		// address. (pool read pad)
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, PC, opc);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 3;
	}
};

class INVOKEVIRTUAL4MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL4MicroInstruction() {
		// PC = the pool entry (the callee address): jump, flush, refill.
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, pc);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 4;
	}
};

class INVOKEVIRTUAL5MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL5MicroInstruction() {
		// H = params_hi << 8 (consumes the first header byte; the
		// refill lands during this cycle's stall/retry).
		ALU_SLL8_A(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 5;
	}
};

class INVOKEVIRTUAL6MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL6MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		REG_READ(inst_, H);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 6;
	}
};

class INVOKEVIRTUAL7MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL7MicroInstruction() {
		ALU_SUB(inst_);
		REG_ASSIGN(inst_, SP, tos);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 7;
	}
};

class INVOKEVIRTUAL8MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL8MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, TOS, mar, tos);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 8;
	}
};

class INVOKEVIRTUAL9MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL9MicroInstruction() {
		ALU_SLL8_A(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 9;
	}
};

class INVOKEVIRTUAL10MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL10MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		REG_READ(inst_, H);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 10;
	}
};

class INVOKEVIRTUAL11MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL11MicroInstruction() {
		ALU_ADD_INC(inst_);
		REG_ASSIGN(inst_, SP, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 11;
	}
};

class INVOKEVIRTUAL12MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL12MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN2(inst_, MDR, mar, sp);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 12;
	}
};

class INVOKEVIRTUAL13MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL13MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, OPC, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 13;
	}
};

class INVOKEVIRTUAL14MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL14MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 14;
	}
};

class INVOKEVIRTUAL15MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL15MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, LV, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 15;
	}
};

class INVOKEVIRTUAL16MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL16MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, lv);
		inst_.next_address = MAIN_ADDR;
	}
};
