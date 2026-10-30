#pragma once

#include <assert.h>
#include <stdint.h>

#define ALU_PASS_B(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::OR; \
	alu.enb = 1; \
	INST.alu = alu;

#define ALU_PASS_A(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::OR; \
	alu.ena = 1; \
	INST.alu = alu;

#define ALU_ADD(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::SUM; \
	alu.ena = 1; \
	alu.enb = 1; \
	INST.alu = alu;

#define ALU_ADD_INC(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::SUM; \
	alu.ena = 1; \
	alu.enb = 1; \
	alu.inc = 1; \
	INST.alu = alu;

#define ALU_SUB(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::SUM; \
	alu.ena = 1; \
	alu.enb = 1; \
	alu.inva = 1; \
	alu.inc = 1; \
	INST.alu = alu;

#define ALU_INC_B(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::SUM; \
	alu.enb = 1; \
	alu.inc = 1; \
	INST.alu = alu;

#define ALU_DEC_B(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::SUM; \
	alu.enb = 1; \
	alu.inva = 1; \
	INST.alu = alu;

#define ALU_SLL8_A(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::OR; \
	alu.ena = 1; \
	alu.sll8 = 1; \
	INST.alu = alu;

#define ALU_AND(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::AND; \
	alu.ena = 1; \
	alu.enb = 1; \
	INST.alu = alu;

#define ALU_OR(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::OR; \
	alu.ena = 1; \
	alu.enb = 1; \
	INST.alu = alu;

enum ALUFunctionTypes {
	AND = 0,
	OR = 1,
	INVB = 2,
	SUM = 3
};

struct ALUFunction {
	uint8_t func : 2;
	uint8_t ena : 1;
	uint8_t enb : 1;
	uint8_t inva : 1;
	uint8_t inc : 1;
	uint8_t sll8 : 1;
	uint8_t sra1 : 1;
};

class ALU {
public:
	// Three-micro-cycle pipeline:
	//   micro-cycle 1: the A/B bus mux values latch into pipeline
	//                  registers A and B (SetAB)
	//   micro-cycle 2: the combinational core runs on A/B; the result
	//                  lands in pipeline register C (Execute + Shift)
	//   micro-cycle 3: C is written back to the register file (GetC)
	ALU() {}
	~ALU() {}

	void SetAB(uint32_t a, uint32_t b) {
		a_ = a;
		b_ = b;
	}

	void SetFunction(ALUFunction f) {
		f_ = f;
	}

	void Reset() {
		SetAB(0, 0);
		SetFunction({ 0 });
		c_ = 0;
		n_ = 0;
		z_ = 0;
	}

	void Execute() {
		uint32_t a = (f_.ena ? a_ : 0);
		uint32_t b = (f_.enb ? b_ : 0);
		if (f_.inva) {
			a = ~a;
		}
		switch (f_.func) {
		case ALUFunctionTypes::AND:
			c_ = a & b;
			break;
		case ALUFunctionTypes::OR:
			c_ = a | b;
			break;
		case ALUFunctionTypes::INVB:
			c_ = ~b;
			break;
		case ALUFunctionTypes::SUM:
			c_ = a + b;
			break;
		default:
			assert(0);
		}
		if (f_.inc) {
			c_ += 1;
		}
		
		uint32_t msb = (c_ >> 31) & 1;
		n_ = (msb == 1) ? 1 : 0;
		z_ = (c_ == 0) ? 1 : 0;
	}

	void Shift() {
		uint32_t msb = (c_ >> 31) & 1;
		if (f_.sll8) {
			c_ <<= 8;
		}
		if (f_.sra1) {
			c_ >>= 1;
			c_ |= (msb << 31);
		}
	}

	uint32_t GetC() {
		return c_;
	}

	uint8_t GetN() {
		return n_;
	}

	uint8_t GetZ() {
		return z_;
	}

private:
	// Pipeline registers: A and B latch the bus values in front of the
	// ALU, C holds the result behind it.
	uint32_t a_ = 0;
	uint32_t b_ = 0;
	ALUFunction f_{ 0 };
	
	uint32_t c_ = 0;
	uint8_t n_ = 0;
	uint8_t z_ = 0;
};