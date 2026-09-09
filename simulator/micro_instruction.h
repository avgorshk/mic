#pragma once

#include <stdint.h>

#include "alu.h"

struct Signals {
	// ALU Signals (8)
	ALUFunction alu_;

	// Write Signals (9)
	uint8_t mar_write_ : 1;
	uint8_t mdr_write_ : 1;
	uint8_t pc_write_ : 1;
	uint8_t sp_write_ : 1;
	uint8_t lv_write_ : 1;
	uint8_t cpp_write_ : 1;
	uint8_t tos_write_ : 1;
	uint8_t opc_write_ : 1;
	uint8_t h_write_ : 1;

	// Read Signals (9)
	uint8_t mdr_read_ : 1;
	uint8_t pc_read_ : 1;
	uint8_t mbr_read_unsigned_ : 1;
	uint8_t mbr_read_signed_ : 1;
	uint8_t sp_read_ : 1;
	uint8_t lv_read_ : 1;
	uint8_t cpp_read_ : 1;
	uint8_t tos_read_ : 1;
	uint8_t opc_read_ : 1;

	// Memory Signals (3)
	uint8_t mem_rd_ : 1;
	uint8_t mem_wr_ : 1;
	uint8_t mem_fetch_ : 1;

	uint8_t reserved_ : 3;
};

class MicroInstruction {
public:
	MicroInstruction(Signals encoding) : encoding_(encoding) {}

    

private:
	Signals encoding_ = { 0 };
};