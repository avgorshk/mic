#pragma once
#include "../micro_instruction.h"


constexpr uint32_t IRETURN_ADDR = 0xAC;

// Structure preserved from the pre-IFU flow; only the fetch machinery is
// replaced: IRETURN5's write to PC now jumps the IFU (flush + refill at
// the return address), and IRETURN8 fuses the trailing dispatch. The
// refill lands while IRETURN6-8 run, so the dispatch never stalls.
class IRETURN1MicroInstruction : public MicroInstruction {
public:
	IRETURN1MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN2(inst_, LV, mar, sp);
		inst_.mem_rd = 1;
		inst_.next_address = IRETURN_ADDR + 1;
	}
};

class IRETURN2MicroInstruction : public MicroInstruction {
public:
	IRETURN2MicroInstruction() {
		inst_.next_address = IRETURN_ADDR + 2;
	}
};

class IRETURN3MicroInstruction : public MicroInstruction {
public:
	IRETURN3MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN2(inst_, MDR, lv, mar);
		inst_.mem_rd = 1;
		inst_.next_address = IRETURN_ADDR + 3;
	}
};

class IRETURN4MicroInstruction : public MicroInstruction {
public:
	IRETURN4MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, LV, mar);
		inst_.next_address = IRETURN_ADDR + 4;
	}
};

class IRETURN5MicroInstruction : public MicroInstruction {
public:
	IRETURN5MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, pc);
		inst_.mem_rd = 1;
		inst_.next_address = IRETURN_ADDR + 5;
	}
};

class IRETURN6MicroInstruction : public MicroInstruction {
public:
	IRETURN6MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, SP, mar);
		inst_.next_address = IRETURN_ADDR + 6;
	}
};

class IRETURN7MicroInstruction : public MicroInstruction {
public:
	IRETURN7MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, lv);
		inst_.next_address = IRETURN_ADDR + 7;
	}
};

class IRETURN8MicroInstruction : public MicroInstruction {
public:
	IRETURN8MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, mdr);
		inst_.mem_wr = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0;
	}
};
