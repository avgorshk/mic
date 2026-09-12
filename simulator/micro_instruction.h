#pragma once

#include "alu.h"
#include "signals.h"

struct MicroInstruction {
	// Addr (9)
	uint16_t next_address : 9;

	// JAM (3)
	uint8_t jmpc : 1;
	uint8_t jamn : 1;
	uint8_t jamz : 1;

	// ALU (8)
	ALUFunction alu;

	// C Bus (9)
	uint8_t write_mar : 1;
	uint8_t write_mdr : 1;
	uint8_t write_pc : 1;
	uint8_t write_sp : 1;
	uint8_t write_lv : 1;
	uint8_t write_cpp : 1;
	uint8_t write_tos : 1;
	uint8_t write_opc : 1;
	uint8_t write_h : 1;

	// Mem (3)
	uint8_t mem_rd : 1;
	uint8_t mem_wr : 1;
	uint8_t mem_fetch : 1;

	// B Bus (4)
	uint8_t read : 4;

	Signals GetSignals() {
		Signals signals;

		// ALU Signals (8)
		signals.alu = alu;

		// Write Signals (9)
		signals.write_mar = write_mar;
		signals.write_mdr = write_mdr;
		signals.write_pc = write_pc;
		signals.write_sp = write_sp;
		signals.write_lv = write_lv;
		signals.write_cpp = write_cpp;
		signals.write_tos = write_tos;
		signals.write_opc = write_opc;
		signals.write_h = write_h;

		// Read Signals (9)
		assert(read < 9);
		switch (read) {
		case 0: {
			signals.read_mdr = 1;
			break;
		}
		case 1: {
			signals.read_pc = 1;
			break;
		}
		case 2: {
			signals.read_mbr_unsigned = 1;
			break;
		}
		case 3: {
			signals.read_mbr_signed = 1;
			break;
		}
		case 4: {
			signals.read_sp = 1;
			break;
		}
		case 5: {
			signals.read_lv = 1;
			break;
		}
		case 6: {
			signals.read_cpp = 1;
			break;
		}
		case 7: {
			signals.read_tos = 1;
			break;
		}
		case 8: {
			signals.read_opc = 1;
			break;
		}
		}

		// Memory Signals (3)
		signals.mem_rd = mem_rd;
		signals.mem_wr = mem_wr;
		signals.mem_fetch = mem_fetch;

		return signals;
	}
};