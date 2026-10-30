#pragma once

#include "alu.h"
#include "signals.h"

constexpr uint16_t HALT_SIGNAL = 0xFE;

// A bus: 11 sources. H = 0 so a pre-rework MIR (a_read = 0) keeps its
// exact old behavior (A was always H).
enum MicroInstructionARead {
	AREAD_H = 0,
	AREAD_MDR = 1,
	AREAD_MBR_SIGNED = 2,
	AREAD_MBR_UNSIGNED = 3,
	AREAD_SP = 4,
	AREAD_LV = 5,
	AREAD_CPP = 6,
	AREAD_TOS = 7,
	AREAD_OPC = 8,
	AREAD_MBR2_SIGNED = 9,
	AREAD_MBR2_UNSIGNED = 10
};

// B bus: 8 sources. MBR is readable via the A bus only.
enum MicroInstructionBRead {
	BREAD_MDR = 0,
	BREAD_SP = 1,
	BREAD_LV = 2,
	BREAD_CPP = 3,
	BREAD_TOS = 4,
	BREAD_OPC = 5,
	BREAD_PC = 6,
	BREAD_H = 7
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

	// Mem (2): the IFU refills itself; instructions no longer fetch.
	uint8_t mem_rd : 1;
	uint8_t mem_wr : 1;

	// A Bus (4), B Bus (3)
	uint8_t a_read : 4;
	uint8_t b_read : 3;
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

		// Bus Read Signals (A: 4 bits, B: 3 bits)
		assert(inst_.a_read <= 10);
		assert(inst_.b_read <= 7);
		signals.a_read = inst_.a_read;
		signals.b_read = inst_.b_read;

		// Memory Signals (2)
		signals.mem_rd = inst_.mem_rd;
		signals.mem_wr = inst_.mem_wr;

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

	bool GetHalt() const {
		return (inst_.next_address == HALT_SIGNAL);
	}

protected:
	MicroInstructionEncoding inst_ = { 0 };
};

#define REG_READ_B(INST, IN) \
	INST.b_read = BREAD_##IN;

#define REG_READ_A(INST, IN) \
	INST.a_read = AREAD_##IN;

#define REG_ASSIGN0(INST, OUT) \
	INST.write_##OUT = 1;

#define REG_ASSIGN(INST, IN, OUT) \
	INST.b_read = BREAD_##IN; \
	INST.write_##OUT = 1;

#define REG_ASSIGN_A(INST, IN, OUT) \
	INST.a_read = AREAD_##IN; \
	INST.write_##OUT = 1;

#define REG_ASSIGN2(INST, IN, OUT1, OUT2) \
	INST.b_read = BREAD_##IN; \
	INST.write_##OUT1 = 1; \
	INST.write_##OUT2 = 1;

#define REG_ASSIGN2_A(INST, IN, OUT1, OUT2) \
	INST.a_read = AREAD_##IN; \
	INST.write_##OUT1 = 1; \
	INST.write_##OUT2 = 1;
