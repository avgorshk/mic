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

	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	mic.SetData(data);
	mic.SetSP(data.size() - 1);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // ILOAD1
	mic.Cycle(); // ILOAD2
	mic.Cycle(); // ILOAD3
	mic.Cycle(); // ILOAD4
	mic.Cycle(); // ILOAD5
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + 4);
	CHECK(regs.pc == 2);
	CHECK(regs.tos == 20);

	auto output = mic.GetData(5);
	CHECK(output[4] == 20);
}
