#pragma once

#include <cassert>
#include <vector>

#include "micro_instruction.h"

constexpr size_t CONTROL_MEMORY_SIZE = 512;

class ControlMemory {
public:
	ControlMemory() : memory_(CONTROL_MEMORY_SIZE) {}

	// TODO: remove
	void SetMicroInstruction(MicroInstruction inst) {
		memory_[0] = inst;
	}

public:
	MicroInstruction LoadMIR() {
		assert(mpc_ < CONTROL_MEMORY_SIZE);
		mir_ = memory_[mpc_];
		return mir_;
	}

	void UpdateMPC(uint8_t n, uint8_t z, uint32_t mbr) {
		mpc_ = mir_.next_address;
		if (mir_.jamn == 1) {
			mpc_ |= (n << 8);
		}
		if (mir_.jamz == 1) {
			mpc_ |= (z << 8);
		}
		if (mir_.jmpc == 1) {
			mpc_ |= (mbr & 0xFF);
		}
	}

private:
	std::vector<MicroInstruction> memory_;
	uint16_t mpc_ = 0;
	MicroInstruction mir_ = { 0 };
};