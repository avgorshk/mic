#pragma once

#include <assert.h>
#include <stdint.h>

#define ALU_ASSIGN_B(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::OR; \
	alu.enb = 1; \
	INST.alu = alu;

#define ALU_ADD(INST) \
	ALUFunction alu = { 0 }; \
	alu.func = ALUFunctionTypes::SUM; \
	alu.ena = 1; \
	alu.enb = 1; \
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
	ALU() {}
	~ALU() {}

	void SetInput(uint32_t a, uint32_t b) {
		a_ = a;
		b_ = b;
	}

	void SetFunction(ALUFunction f) {
		f_ = f;
	}

	void Reset() {
		SetInput(0, 0);
		SetFunction({ 0 });
		result_ = 0;
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
			result_ = a & b;
			break;
		case ALUFunctionTypes::OR:
			result_ = a | b;
			break;
		case ALUFunctionTypes::INVB:
			result_ = ~b;
			break;
		case ALUFunctionTypes::SUM:
			result_ = a + b;
			break;
		default:
			assert(0);
		}
		if (f_.inc) {
			result_ += 1;
		}
		
		if (result_ == 0) {
			z_ = 1;
		}

		uint32_t msb = (result_ >> 31) & 1;
		if (msb == 1) {
			n_ = 1;
		}
	}

	void Shift() {
		uint32_t msb = (result_ >> 31) & 1;
		if (f_.sll8) {
			result_ <<= 8;
		}
		if (f_.sra1) {
			result_ >>= 1;
			result_ |= (msb << 31);
		}
	}

	uint32_t GetResult() {
		return result_;
	}

	uint8_t GetN() {
		return n_;
	}

	uint8_t GetZ() {
		return z_;
	}

private:
	uint32_t a_ = 0;
	uint32_t b_ = 0;
	ALUFunction f_{ 0 };
	
	uint32_t result_ = 0;
	uint8_t n_ = 0;
	uint8_t z_ = 0;
};