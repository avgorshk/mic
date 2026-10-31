#pragma once
#include "../micro_instruction.h"


constexpr uint32_t INVOKEVIRTUAL_ADDR = 0xB4;

// 16 cycles (was 22). The index comes straight from MBR2 (no 3-cycle
// assembly), the jump to the callee is write_pc (flush + refill, the
// decoder resyncs at the landing), and the four header bytes are
// consumed directly from the refilled queue: each MBR read advances the
// queue, so the fetch cycles and their waits collapse away. IV5's
// delivery waits once while the first callee block is in flight (the
// byte-availability gate at the queue -> MIR1 handoff) -- the honest
// cost of the jump.
class INVOKEVIRTUAL1MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL1MicroInstruction() {
		ALU_PASS_A(inst_);
		REG_ASSIGN_A(inst_, MBR2_UNSIGNED, h);
	}
};

class INVOKEVIRTUAL2MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL2MicroInstruction() {
		ALU_ADD(inst_);
		REG_READ_A(inst_, H);
		REG_READ_B(inst_, H);
		REG_ASSIGN(inst_, CPP, mar);
		inst_.mem_rd = 1;
	}
};

class INVOKEVIRTUAL3MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL3MicroInstruction() {
		// The IFU's PC is already past the 2-byte index: the return
		// address. (pool read pad)
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, PC, opc);
	}
};

class INVOKEVIRTUAL4MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL4MicroInstruction() {
		// PC = the pool entry (the callee address): jump, flush, refill.
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, pc);
	}
};

class INVOKEVIRTUAL5MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL5MicroInstruction() {
		// H = params_hi << 8 (consumes the first header byte; the
		// refill lands during this MIR's delivery wait).
		ALU_SLL8_A(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
	}
};

class INVOKEVIRTUAL6MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL6MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		REG_READ_B(inst_, H);
	}
};

class INVOKEVIRTUAL7MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL7MicroInstruction() {
		ALU_SUB(inst_);
		REG_READ_A(inst_, H);
		REG_ASSIGN(inst_, SP, tos);
	}
};

class INVOKEVIRTUAL8MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL8MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, TOS, mar, tos);
	}
};

class INVOKEVIRTUAL9MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL9MicroInstruction() {
		ALU_SLL8_A(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
	}
};

class INVOKEVIRTUAL10MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL10MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		REG_READ_B(inst_, H);
	}
};

class INVOKEVIRTUAL11MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL11MicroInstruction() {
		ALU_ADD_INC(inst_);
		REG_READ_A(inst_, H);
		REG_ASSIGN(inst_, SP, mdr);
		inst_.mem_wr = 1;
	}
};

class INVOKEVIRTUAL12MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL12MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN2(inst_, MDR, mar, sp);
	}
};

class INVOKEVIRTUAL13MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL13MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, OPC, mdr);
		inst_.mem_wr = 1;
	}
};

class INVOKEVIRTUAL14MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL14MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
	}
};

class INVOKEVIRTUAL15MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL15MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, LV, mdr);
		inst_.mem_wr = 1;
	}
};

class INVOKEVIRTUAL16MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL16MicroInstruction() {
		// LV = TOS. The end bit lets the queue confirm the resynced
		// decoder (the callee's opcode has been at the queue front
		// since the header bytes were consumed by IV5/IV6/IV9/IV10).
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, lv);
		END_MIR(inst_);
	}
};
