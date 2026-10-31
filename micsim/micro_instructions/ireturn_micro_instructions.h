#pragma once
#include "../micro_instruction.h"


constexpr uint32_t IRETURN_ADDR = 0xAC;

// Structure preserved from the pre-IFU flow; only the control machinery
// changed: IRETURN5's write to PC jumps the IFU (flush + refill at the
// return address, the decoder resyncs at the landing), and the trailing
// dispatch is gone -- the queue confirms the resynced decoder after the
// end bit. The refill lands while IRETURN6-8 run, so no stall is
// possible.
class IRETURN1MicroInstruction : public MicroInstruction {
public:
	IRETURN1MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN2(inst_, LV, mar, sp);
		inst_.mem_rd = 1;
	}
};

class IRETURN2MicroInstruction : public MicroInstruction {
public:
	IRETURN2MicroInstruction() {
	}
};

class IRETURN3MicroInstruction : public MicroInstruction {
public:
	IRETURN3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN2(inst_, MDR, lv, mar);
		inst_.mem_rd = 1;
	}
};

class IRETURN4MicroInstruction : public MicroInstruction {
public:
	IRETURN4MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, LV, mar);
	}
};

class IRETURN5MicroInstruction : public MicroInstruction {
public:
	IRETURN5MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, pc);
		inst_.mem_rd = 1;
	}
};

class IRETURN6MicroInstruction : public MicroInstruction {
public:
	IRETURN6MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, SP, mar);
	}
};

class IRETURN7MicroInstruction : public MicroInstruction {
public:
	IRETURN7MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, lv);
	}
};

class IRETURN8MicroInstruction : public MicroInstruction {
public:
	IRETURN8MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, mdr);
		inst_.mem_wr = 1;
		END_MIR(inst_);
	}
};
