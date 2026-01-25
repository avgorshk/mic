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

struct Signals {
	uint8_t mar_write_ : 1;
	uint8_t mdr_write_ : 1;
	uint8_t mdr_read_ : 1;
	uint8_t pc_write_ : 1;
	uint8_t pc_read_ : 1;
	uint8_t mbr_read_unsigned_ : 1;
	uint8_t mbr_read_signed_ : 1;
	uint8_t sp_write_ : 1;
	uint8_t sp_read_ : 1;
	uint8_t lv_write_ : 1;
	uint8_t lv_read_ : 1;
	uint8_t cpp_write_ : 1;
	uint8_t cpp_read_ : 1;
	uint8_t tos_write_ : 1;
	uint8_t tos_read_ : 1;
	uint8_t opc_write_ : 1;
	uint8_t opc_read_ : 1;
	uint8_t h_write_ : 1;
	uint8_t reserved0 : 5;
	uint8_t reserved1;
};

class MIC {
public:
	void Run() {

	}

private:
	void Step() {
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
	Signals signals_{ 0 };
	Registers regs_;
	ALU alu_;
	ControlMemory control_memory_;
};