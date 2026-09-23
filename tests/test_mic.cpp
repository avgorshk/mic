#include "doctest.h"

#include "mic.h"
#include "micro_instructions/micro_instructions.h"

TEST_CASE("Signals Size") {
	CHECK(sizeof(Signals) == 4);
}

TEST_CASE("ILOAD") {
	MIC mic;

	std::vector<uint8_t> program = { ILOAD_ADDR, 0x2 };
	mic.SetProgram(program);

	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	mic.SetData(data);
	mic.SetSP(static_cast<uint32_t>(data.size()) - 1);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // ILOAD1
	mic.Cycle(); // ILOAD2
	mic.Cycle(); // ILOAD3
	mic.Cycle(); // ILOAD4
	mic.Cycle(); // ILOAD5
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + data.size());
	CHECK(regs.pc == 2);
	CHECK(regs.tos == data[2]);

	auto output = mic.GetData(data.size() + 1);
	CHECK(output[data.size()] == regs.tos);
}

TEST_CASE("IADD") {
	MIC mic;

	std::vector<uint8_t> program = { IADD_ADDR };
	mic.SetProgram(program);

	std::vector<uint32_t> data = { 0, 10, 20, 30, 20, 30 };
	mic.SetData(data);
	mic.SetSP(static_cast<uint32_t>(data.size()) - 1);
	mic.SetTOS(data[data.size() - 1]);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // IADD1
	mic.Cycle(); // IADD2
	mic.Cycle(); // IADD3
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + data.size() - 2);
	CHECK(regs.pc == 2);
	CHECK(regs.tos == data[4] + data[5]);

	auto output = mic.GetData(data.size());
	CHECK(output[data.size() - 2] == regs.tos);
}
