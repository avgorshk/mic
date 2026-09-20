#include "doctest.h"

#include "mic.h"
#include "micro_instructions/micro_instructions.h"

TEST_CASE("Signals Size") {
	CHECK(sizeof(Signals) == 4);
}

TEST_CASE("IADD") {
	MIC mic;

	std::vector<uint8_t> program = { ILOAD_ADDR, 0x2 };
	mic.SetProgram(program);

	std::vector<uint32_t> data = { 0, 1, 2, 3 };
	mic.SetData(data);
	mic.SetSP(data.size() - 1);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // ILOAD1
	mic.Cycle(); // ILOAD2
	mic.Cycle(); // ILOAD3
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == 1);
}
