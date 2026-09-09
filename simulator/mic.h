#pragma once

#include "alu.h"
#include "control_memory.h"

struct Registers {
	uint32_t mar_ = 0;
	uint32_t mdr_ = 0;
	uint32_t pc_ = 0;
	uint32_t mbr_ = 0;
	uint32_t sp_ = 0;
	uint32_t lv_ = 0;
	uint32_t cpp_ = 0;
	uint32_t tos_ = 0;
	uint32_t opc_ = 0;
	uint32_t h_ = 0;
};

class MIC {
public:
	void Run() {

	}

private:
	void Cycle() {
		SetControlSignals();
		ReadRegisters();
		RunALU();
		WriteRegisters();
	}

	void SetControlSignals() {

	}

	void ReadRegisters() {

	}

	void RunALU() {

	}

	void WriteRegisters() {

	}

private:
	Registers regs_;
	ALU alu_;
	ControlMemory control_memory_;
};