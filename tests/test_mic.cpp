#include "doctest.h"

#include "global_memory.h"
#include "mic.h"
#include "micro_instructions/micro_instructions.h"

constexpr uint32_t PROGRAM_SEGMENT_SIZE = 1024;   // 1 KB
constexpr uint32_t CONSTANT_SEGMENT_SIZE = 1024;  // 1 KB
constexpr uint32_t DATA_SEGMENT_SIZE = 2014;      // 1 KB

TEST_CASE("Signals Size") {
	CHECK(sizeof(Signals) == 4);
}

TEST_CASE("ILOAD") {
	std::vector<uint8_t> program = { ILOAD_ADDR, 0x2 };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

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

	auto output = memory.GetData(data.size() + 1);
	CHECK(output[data.size()] == regs.tos);
}

TEST_CASE("WIDE_ILOAD") {
	std::vector<uint8_t> program = { WIDE_ADDR, ILOAD_ADDR, 0x0, 0x2 };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // WIDE
	mic.Cycle(); // WIDE_ILOAD1
	mic.Cycle(); // WIDE_ILOAD2
	mic.Cycle(); // WIDE_ILOAD3
	mic.Cycle(); // WIDE_ILOAD4
	mic.Cycle(); // ILOAD3
	mic.Cycle(); // ILOAD4
	mic.Cycle(); // ILOAD5
	auto regs = mic.GetRegisters();

	auto varnum = (program[2] << 8) | program[3];
	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[varnum]);

	auto output = memory.GetData(data.size() + 1);
	CHECK(output[data.size()] == regs.tos);
}

TEST_CASE("IADD") {
	std::vector<uint8_t> program = { IADD_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

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

	auto output = memory.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("ISUB") {
	std::vector<uint8_t> program = { ISUB_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // ISUB1
	mic.Cycle(); // ISUB2
	mic.Cycle(); // ISUB3
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size() + 1);
	CHECK(regs.tos == data[sp - 1] - data[sp]);

	auto output = memory.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("IAND") {
	std::vector<uint8_t> program = { IAND_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // ISUB1
	mic.Cycle(); // ISUB2
	mic.Cycle(); // ISUB3
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size() + 1);
	CHECK(regs.tos == (data[sp - 1] & data[sp]));

	auto output = memory.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("IOR") {
	std::vector<uint8_t> program = { IOR_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // ISUB1
	mic.Cycle(); // ISUB2
	mic.Cycle(); // ISUB3
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size() + 1);
	CHECK(regs.tos == (data[sp - 1] | data[sp]));

	auto output = memory.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("DUP") {
	std::vector<uint8_t> program = { DUP_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // DUP1
	mic.Cycle(); // DUP2
	mic.Cycle(); // MAIN
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size() + 1);

	auto output = memory.GetData(data.size() + 1);
	CHECK(output[sp + 1] == data[sp]);
}

TEST_CASE("POP") {
	std::vector<uint8_t> program = { POP_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // POP1
	mic.Cycle(); // POP2
	mic.Cycle(); // POP3
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1]);
}

TEST_CASE("SWAP") {
	std::vector<uint8_t> program = { SWAP_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 40, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // SWAP1
	mic.Cycle(); // SWAP2
	mic.Cycle(); // SWAP3
	mic.Cycle(); // SWAP4
	mic.Cycle(); // SWAP5
	mic.Cycle(); // SWAP6
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1]);

	auto output = memory.GetData(data.size());
	CHECK(output[sp] == data[sp - 1]);
	CHECK(output[sp - 1] == data[sp]);
}

TEST_CASE("ISTORE") {
	std::vector<uint8_t> program = { ISTORE_ADDR, 0x1 };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

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

	auto output = memory.GetData(data.size());
	CHECK(output[program[1]] == data[sp]);
}

TEST_CASE("BIPUSH") {
	std::vector<uint8_t> program = { BIPUSH_ADDR, 0x7 };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

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

	auto output = memory.GetData(data.size() + 1);
	CHECK(output[sp + 1] == program[1]);
}

TEST_CASE("IF_ICMPEQ Equal") {
	std::vector<uint8_t> program = { IF_ICMPEQ_ADDR, 0x1, 0xAB, GOTO_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 50, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // T
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 2);
	CHECK(regs.pc == ((program[1] << 8) | program[2]));
	CHECK(regs.tos == data[sp - 2]);
}

TEST_CASE("IF_ICMPEQ Nonequal") {
	std::vector<uint8_t> program = { IF_ICMPEQ_ADDR, 0x1, 0xAB, GOTO_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 50, 60 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // IF_ICMPEQ
	mic.Cycle(); // F
	mic.Cycle(); // F
	mic.Cycle(); // F
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 2);
	CHECK(regs.pc == 3);
	CHECK(regs.mbr == GOTO_ADDR);
	CHECK(regs.tos == data[sp - 2]);
}

TEST_CASE("GOTO") {
	std::vector<uint8_t> program = { GOTO_ADDR, 0x1, 0xAB, GOTO_ADDR };
	
	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);

	MIC mic(&memory);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	mic.Cycle(); // GOTO
	auto regs = mic.GetRegisters();

	CHECK(regs.pc == ((program[1] << 8) | program[2]));
}

TEST_CASE("Program Equal") {
	std::vector<uint32_t> data = { 0, 0, 1, 2 };
	std::vector<uint8_t> program = {
		ILOAD_ADDR, 0x02,
		ILOAD_ADDR, 0x03,
		IADD_ADDR,
		ISTORE_ADDR, 0x01,
		ILOAD_ADDR, 0x01,
		BIPUSH_ADDR, 0x03,
		IF_ICMPEQ_ADDR, 0x00, 0x0D,
		ILOAD_ADDR, 0x02,
		BIPUSH_ADDR, 0x01,
		ISUB_ADDR,
		ISTORE_ADDR, 0x02,
		GOTO_ADDR, 0x00, 0x07,
		BIPUSH_ADDR, 0x00,
		ISTORE_ADDR, 0x03,
		HALT_ADDR
	};
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

	while (true) {
		if (mic.Cycle()) break;
	}

	auto output = memory.GetData(data.size());
	CHECK(output[3] == 0);
}

TEST_CASE("Program Nonequal") {
	std::vector<uint32_t> data = { 0, 0, 5, 7 };
	std::vector<uint8_t> program = {
		ILOAD_ADDR, 0x02,
		ILOAD_ADDR, 0x03,
		IADD_ADDR,
		ISTORE_ADDR, 0x01,
		ILOAD_ADDR, 0x01,
		BIPUSH_ADDR, 0x03,
		IF_ICMPEQ_ADDR, 0x00, 0x0D,
		ILOAD_ADDR, 0x02,
		BIPUSH_ADDR, 0x01,
		ISUB_ADDR,
		ISTORE_ADDR, 0x02,
		GOTO_ADDR, 0x00, 0x07,
		BIPUSH_ADDR, 0x00,
		ISTORE_ADDR, 0x03,
		HALT_ADDR
	};
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

	while (true) {
		if (mic.Cycle()) break;
	}

	auto output = memory.GetData(data.size());
	CHECK(output[2] == 4);
}

TEST_CASE("Program Equal Wide Load") {
	std::vector<uint32_t> data = { 0, 0, 1, 2 };
	std::vector<uint8_t> program = {
		ILOAD_ADDR, 0x02,
		ILOAD_ADDR, 0x03,
		IADD_ADDR,
		ISTORE_ADDR, 0x01,
		WIDE_ADDR, ILOAD_ADDR, 0x00, 0x01,
		BIPUSH_ADDR, 0x03,
		IF_ICMPEQ_ADDR, 0x00, 0x0D,
		ILOAD_ADDR, 0x02,
		BIPUSH_ADDR, 0x01,
		ISUB_ADDR,
		ISTORE_ADDR, 0x02,
		GOTO_ADDR, 0x00, 0x07,
		BIPUSH_ADDR, 0x00,
		ISTORE_ADDR, 0x03,
		HALT_ADDR
	};
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

	while (true) {
		if (mic.Cycle()) break;
	}

	auto output = memory.GetData(data.size());
	CHECK(output[3] == 0);
}