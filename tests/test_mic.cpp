#include "doctest.h"

#include "mic.h"
#include "micro_instructions/micro_instructions.h"

TEST_CASE("Signals Size") {
	CHECK(sizeof(Signals) == 4);
}

TEST_CASE("IADD") {
	MIC mic;
	std::vector<uint8_t> program = { IADD_ADDR };

	mic.SetProgram(program);
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == 1);
}
