#include "doctest.h"

#include "mic.h"
#include "micro_instructions/micro_instructions.h"

TEST_CASE("Signals Size") {
	CHECK(sizeof(Signals) == 4);
}

TEST_CASE("ILOAD") {
	MIC mic;

	std::vector<uint8_t> program = { ILOAD_ADDR, 0x2 };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	mic.SetProgram(program);
	mic.SetData(data);
	mic.SetSP(sp);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // ILOAD1
	mic.Cycle(); // ILOAD2
	mic.Cycle(); // ILOAD3
	mic.Cycle(); // ILOAD4
	mic.Cycle(); // ILOAD5
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[program[1]]);

	auto output = mic.GetData(data.size() + 1);
	CHECK(output[data.size()] == regs.tos);
}

TEST_CASE("IADD") {
	MIC mic;

	std::vector<uint8_t> program = { IADD_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	mic.SetProgram(program);
	mic.SetData(data);
	mic.SetSP(sp);
	mic.SetTOS(data[sp]);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // IADD1
	mic.Cycle(); // IADD2
	mic.Cycle(); // IADD3
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size() + 1);
	CHECK(regs.tos == data[sp - 1] + data[sp]);

	auto output = mic.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("ISTORE") {
	MIC mic;

	std::vector<uint8_t> program = { ISTORE_ADDR, 0x1 };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	mic.SetProgram(program);
	mic.SetData(data);
	mic.SetSP(sp);
	mic.SetTOS(data[sp]);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // ISTORE1
	mic.Cycle(); // ISTORE2
	mic.Cycle(); // ISTORE3
	mic.Cycle(); // ISTORE4
	mic.Cycle(); // ISTORE5
	mic.Cycle(); // ISTORE6
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1]);

	auto output = mic.GetData(data.size());
	CHECK(output[program[1]] == data[sp]);
}

TEST_CASE("BIPUSH") {
	MIC mic;

	std::vector<uint8_t> program = { BIPUSH_ADDR, 0x7 };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	mic.SetProgram(program);
	mic.SetData(data);
	mic.SetSP(sp);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // BIPUSH
	mic.Cycle(); // BIPUSH
	mic.Cycle(); // BIPUSH
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size() + 1);
	CHECK(regs.tos == program[1]);

	auto output = mic.GetData(data.size() + 1);
	CHECK(output[sp + 1] == program[1]);
}

TEST_CASE("IF_ICMPEQ Equal") {
	MIC mic;

	std::vector<uint8_t> program = { IF_ICMPEQ_ADDR, 0xF };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 50, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	mic.SetProgram(program);
	mic.SetData(data);
	mic.SetSP(sp);
	mic.SetTOS(data[sp]);

	mic.InitCycle();
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size() + 1);
	CHECK(regs.tos == program[1]);

	auto output = mic.GetData(data.size() + 1);
	CHECK(output[sp + 1] == program[1]);
}
