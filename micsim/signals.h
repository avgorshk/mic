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

	// Read Signals (9)
	uint8_t read_mdr : 1;
	uint8_t read_pc : 1;
	uint8_t read_mbr_unsigned : 1;
	uint8_t read_mbr_signed : 1;
	uint8_t read_sp : 1;
	uint8_t read_lv : 1;
	uint8_t read_cpp : 1;
	uint8_t read_tos : 1;
	uint8_t read_opc : 1;

	// Memory Signals (3)
	uint8_t mem_rd : 1;
	uint8_t mem_wr : 1;
	uint8_t mem_fetch : 1;
};