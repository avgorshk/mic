#pragma once

#include <cassert>
#include <vector>

#include "micro_instructions/micro_instructions.h"

constexpr size_t CONTROL_MEMORY_SIZE = 512;

class ControlMemory {
public:
	ControlMemory() : memory_(CONTROL_MEMORY_SIZE) {
		memory_[NOP_ADDR] = NOPMicroInstruction();
		
		memory_[IADD_ADDR] = IADD1MicroInstruction();

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