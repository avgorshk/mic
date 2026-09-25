#pragma once

#include "alu.h"
#include "signals.h"

enum MicroInstructionRead {
	READ_MDR = 1,
	READ_PC = 2,
	READ_MBR_UNSIGNED = 3,
	READ_MBR_SIGNED = 4,
	READ_SP = 5,
	READ_LV = 6,
	READ_CPP = 7,
	READ_TOS = 8,
	READ_OPC = 9
};

struct MicroInstructionEncoding {
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
};

class MicroInstruction {
public:
	Signals GetSignals() const {
		Signals signals = { 0 };

		// ALU Signals (8)
		signals.alu = inst_.alu;

		// Write Signals (9)
		signals.write_mar = inst_.write_mar;
		signals.write_mdr = inst_.write_mdr;
		signals.write_pc = inst_.write_pc;
		signals.write_sp = inst_.write_sp;
		signals.write_lv = inst_.write_lv;
		signals.write_cpp = inst_.write_cpp;
		signals.write_tos = inst_.write_tos;
		signals.write_opc = inst_.write_opc;
		signals.write_h = inst_.write_h;

		// Read Signals (9)
		assert(inst_.read <= 9);
		switch (inst_.read) {
		case READ_MDR: {
			signals.read_mdr = 1;
			break;
		}
		case READ_PC: {
			signals.read_pc = 1;
			break;
		}
		case READ_MBR_UNSIGNED: {
			signals.read_mbr_unsigned = 1;
			break;
		}
		case READ_MBR_SIGNED: {
			signals.read_mbr_signed = 1;
			break;
		}
		case READ_SP: {
			signals.read_sp = 1;
			break;
		}
		case READ_LV: {
			signals.read_lv = 1;
			break;
		}
		case READ_CPP: {
			signals.read_cpp = 1;
			break;
		}
		case READ_TOS: {
			signals.read_tos = 1;
			break;
		}
		case READ_OPC: {
			signals.read_opc = 1;
			break;
		}
		}

		// Memory Signals (3)
		signals.mem_rd = inst_.mem_rd;
		signals.mem_wr = inst_.mem_wr;
		signals.mem_fetch = inst_.mem_fetch;

		return signals;
	}

	uint16_t GetNextAddress() const {
		return inst_.next_address;
	}

	uint8_t GetJAMN() const {
		return inst_.jamn;
	}

	uint8_t GetJAMZ() const {
		return inst_.jamz;
	}

	uint8_t GetJMPC() const {
		return inst_.jmpc;
	}

protected:
	MicroInstructionEncoding inst_ = { 0 };
};

#define REG_ASSIGN(INST, IN, OUT) \
	INST.read = READ_##IN; \
	INST.write_##OUT = 1;

#define REG_ASSIGN2(INST, IN, OUT1, OUT2) \
	INST.read = READ_##IN; \
	INST.write_##OUT1 = 1; \
	INST.write_##OUT2 = 1;
