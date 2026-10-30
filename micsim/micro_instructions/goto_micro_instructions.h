#pragma once

#include "main_micro_instruction.h"

constexpr uint32_t GOTO_ADDR = 0xA6;

class GOTO1MicroInstruction : public MicroInstruction {
public:
	GOTO1MicroInstruction() {
		// The IFU's PC is the address of the byte at the queue front
		// (the first offset byte), so PC - 1 is the goto's own address.
		ALU_DEC_B(inst_);
		REG_ASSIGN(inst_, PC, opc);
		inst_.next_address = GOTO_ADDR + 1;
	}
};

class GOTO2MicroInstruction : public MicroInstruction {
public:
	GOTO2MicroInstruction() {
		// Target = OPC + sign-extended 16-bit offset; the MBR2 read
		// consumes the offset, then write_pc flushes the queue and
		// refills from the target. The trailing MAIN stalls until the
		// first target block lands and dispatches it.
		ALU_ADD(inst_);
		REG_READ_A(inst_, MBR2_SIGNED);
		REG_ASSIGN(inst_, OPC, pc);
		inst_.next_address = MAIN_ADDR;
	}
};
