#pragma once

#include <cassert>
#include <vector>

#include "micro_instruction.h"

constexpr size_t CONTROL_MEMORY_SIZE = 512;

class ControlMemory {
public:
	ControlMemory() : memory_(CONTROL_MEMORY_SIZE) {}

public:
	MicroInstruction LoadMIR() {
		assert(mpc_ < CONTROL_MEMORY_SIZE);
		mir_ = memory_[mpc_];
		return mir_;
	}

	void UpdateMPC(uint8_t n, uint8_t z) {
		mpc_ = mir_.next_address;
		if (mir_.jamn == 1) {

		}
	}

private:
	std::vector<MicroInstruction> memory_;
	uint16_t mpc_ = 0;
	MicroInstruction mir_ = { 0 };
};