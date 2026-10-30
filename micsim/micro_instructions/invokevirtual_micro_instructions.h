#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t INVOKEVIRTUAL_ADDR = 0xB4;

class INVOKEVIRTUAL1MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL1MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 1;
	}
};

class INVOKEVIRTUAL2MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL2MicroInstruction() {
		ALU_SLL8_A(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 2;
	}
};

class INVOKEVIRTUAL3MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL3MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		REG_READ(inst_, H);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 3;
	}
};

class INVOKEVIRTUAL4MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL4MicroInstruction() {
		ALU_ADD(inst_);
		REG_ASSIGN(inst_, CPP, mar);
		inst_.mem_rd = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 4;
	}
};

class INVOKEVIRTUAL5MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL5MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, opc);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 5;
	}
};

class INVOKEVIRTUAL6MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL6MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, MDR, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 6;
	}
};

class INVOKEVIRTUAL7MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL7MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 7;
	}
};

class INVOKEVIRTUAL8MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL8MicroInstruction() {
		ALU_SLL8_A(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 8;
	}
};

class INVOKEVIRTUAL9MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL9MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		REG_READ(inst_, H);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 9;
	}
};

class INVOKEVIRTUAL10MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL10MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 10;
	}
};

class INVOKEVIRTUAL11MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL11MicroInstruction() {
		ALU_SUB(inst_);
		REG_ASSIGN(inst_, SP, tos);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 11;
	}
};

class INVOKEVIRTUAL12MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL12MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, TOS, mar, tos);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 12;
	}
};

class INVOKEVIRTUAL13MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL13MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 13;
	}
};

class INVOKEVIRTUAL14MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL14MicroInstruction() {
		ALU_SLL8_A(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 14;
	}
};

class INVOKEVIRTUAL15MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL15MicroInstruction() {
		ALU_OR(inst_);
		REG_ASSIGN_A(inst_, MBR_UNSIGNED, h);
		REG_READ(inst_, H);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 15;
	}
};

class INVOKEVIRTUAL16MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL16MicroInstruction() {
		ALU_ADD_INC(inst_);
		REG_ASSIGN(inst_, SP, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 16;
	}
};

class INVOKEVIRTUAL17MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL17MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN2(inst_, MDR, mar, sp);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 17;
	}
};

class INVOKEVIRTUAL18MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL18MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, OPC, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 18;
	}
};

class INVOKEVIRTUAL19MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL19MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN2(inst_, SP, mar, sp);
		inst_.next_address = INVOKEVIRTUAL_ADDR + 19;
	}
};

class INVOKEVIRTUAL20MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL20MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, LV, mdr);
		inst_.mem_wr = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 20;
	}
};

class INVOKEVIRTUAL21MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL21MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.next_address = INVOKEVIRTUAL_ADDR + 21;
	}
};

class INVOKEVIRTUAL22MicroInstruction : public MicroInstruction {
public:
	INVOKEVIRTUAL22MicroInstruction() {
		ALU_PASS_B(inst_);
		REG_ASSIGN(inst_, TOS, lv);
		inst_.next_address = MAIN_ADDR;
	}
};