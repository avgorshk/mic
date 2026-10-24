#pragma once

#include "../micro_instruction.h"

constexpr uint32_t WIDE_ADDR = 0xCA;

// WIDE takes two cycles. The widened opcode is fetched by the MAIN that
// dispatched WIDE; it lands in MBR only at the END of this first (wait)
// cycle. The dispatch in WIDE2 uses the pre-edge MBR, so it can only see
// the operand one cycle later -- exactly what real synchronous hardware
// pays.
class WIDE1MicroInstruction : public MicroInstruction {
public:
	WIDE1MicroInstruction() {
		inst_.next_address = WIDE_ADDR + 1;
	}
};

class WIDE2MicroInstruction : public MicroInstruction {
public:
	WIDE2MicroInstruction() {
		ALU_INC_B(inst_);
		REG_ASSIGN(inst_, PC, pc);
		inst_.mem_fetch = 1;
		inst_.jmpc = 1;
		inst_.next_address = 0x100;
	}
};