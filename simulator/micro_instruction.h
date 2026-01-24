#pragma once

#include <stdint.h>

struct MicroInstructionEncoding {
	uint8_t read_signals_ : 4;
	uint8_t mem_fetch_ : 1;
	uint8_t mem_read_ : 1;
	uint8_t mem_write_ : 1;
	uint8_t mar_write_ : 1;
	uint8_t mdr_write_ : 1;
	uint8_t pc_write_ : 1;
	uint8_t sp_write_ : 1;
	uint8_t lv_write_ : 1;
	uint8_t cpp_write_ : 1;
	uint8_t tos_write_ : 1;
	uint8_t opc_write_ : 1;
	uint8_t h_write_ : 1;
	uint8_t alu_inc : 1;
	uint8_t alu_inva : 1;
	uint8_t alu_enb : 1;
	uint8_t alu_ena : 1;
	uint8_t alu_func : 2;
	uint8_t alu_sra1 : 1;
	uint8_t alu_sll8 : 1;
	uint8_t jamz : 1;
	uint8_t jamn : 1;
	uint8_t jmpc : 1;
	uint8_t reserved0 : 5;
	uint16_t next_addr : 9;
	uint16_t reserved1 : 7;
	uint16_t reserved2;
};

class MicroInstruction {
public:
	MicroInstruction() {}

private:
	
};