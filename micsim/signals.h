#pragma once

#include <stdint.h>

#include "alu.h"

// 32 bits
struct Signals {
	// ALU Signals (8)
	ALUFunction alu;

	// Write Signals (9)
	uint8_t write_mar : 1;
	uint8_t write_mdr : 1;
	uint8_t write_pc : 1;
	uint8_t write_sp : 1;
	uint8_t write_lv : 1;
	uint8_t write_cpp : 1;
	uint8_t write_tos : 1;
	uint8_t write_opc : 1;
	uint8_t write_h : 1;

	// A Bus (4): source for the ALU A input (9 variants, H = 0 default)
	uint8_t a_read : 4;

	// B Bus (3): source for the ALU B input (8 variants)
	uint8_t b_read : 3;

	// Memory Signals (3)
	uint8_t mem_rd : 1;
	uint8_t mem_wr : 1;
	uint8_t mem_fetch : 1;
};