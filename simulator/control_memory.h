#pragma once

#include <cassert>
#include <vector>

#include "micro_instructions/micro_instructions.h"

constexpr size_t CONTROL_MEMORY_SIZE = 512;

class ControlMemory {
public:
	ControlMemory() : memory_(CONTROL_MEMORY_SIZE) {
		memory_[NOP_ADDR] = NOPMicroInstruction();
		
		memory_[BIPUSH_ADDR] = BIPUSH1MicroInstruction();
		memory_[BIPUSH_ADDR + 1] = BIPUSH2MicroInstruction();
		memory_[BIPUSH_ADDR + 2] = BIPUSH3MicroInstruction();

		memory_[ILOAD_ADDR] = ILOAD1MicroInstruction();
		memory_[ILOAD_ADDR + 1] = ILOAD2MicroInstruction();
		memory_[ILOAD_ADDR + 2] = ILOAD3MicroInstruction();
		memory_[ILOAD_ADDR + 3] = ILOAD4MicroInstruction();
		memory_[ILOAD_ADDR + 4] = ILOAD5MicroInstruction();

		memory_[ISTORE_ADDR] = ISTORE1MicroInstruction();
		memory_[ISTORE_ADDR + 1] = ISTORE2MicroInstruction();
		memory_[ISTORE_ADDR + 2] = ISTORE3MicroInstruction();
		memory_[ISTORE_ADDR + 3] = ISTORE4MicroInstruction();
		memory_[ISTORE_ADDR + 4] = ISTORE5MicroInstruction();
		memory_[ISTORE_ADDR + 5] = ISTORE6MicroInstruction();

		memory_[IADD_ADDR] = IADD1MicroInstruction();
		memory_[IADD_ADDR + 1] = IADD2MicroInstruction();
		memory_[IADD_ADDR + 2] = IADD3MicroInstruction();

		memory_[MAIN_ADDR] = MAINMicroInstruction();
	}

public:
	MicroInstruction LoadMIR() {
		assert(mpc_ < CONTROL_MEMORY_SIZE);
		mir_ = memory_[mpc_];
		return mir_;
	}

	void UpdateMPC(uint8_t n, uint8_t z, uint32_t mbr) {
		mpc_ = mir_.GetNextAddress();
		if (mir_.GetJAMN() == 1) {
			mpc_ |= (n << 8);
		}
		if (mir_.GetJAMZ() == 1) {
			mpc_ |= (z << 8);
		}
		if (mir_.GetJMPC() == 1) {
			mpc_ |= (mbr & 0xFF);
		}
	}

private:
	std::vector<MicroInstruction> memory_;
	uint16_t mpc_ = 0;
	MicroInstruction mir_;
};