#pragma once

#include <vector>

#include "micro_instruction.h"

constexpr size_t CONTROL_MEMORY_SIZE = 512;

class ControlMemory {
public: 
	//ControlMemory() : memory_(CONTROL_MEMORY_SIZE) {}

private:
	//std::vector<MicroInstruction> memory_;
	uint16_t mpc_ = 0;
	MicroInstruction mir_;
};