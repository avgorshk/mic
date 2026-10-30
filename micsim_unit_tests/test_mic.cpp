#include "doctest.h"

#include "global_memory.h"
#include "mic.h"
#include "micro_instructions/micro_instructions.h"

constexpr uint32_t PROGRAM_SEGMENT_SIZE = 1024;   // 1 KB
constexpr uint32_t CONSTANT_SEGMENT_SIZE = 1024;  // 1 KB
constexpr uint32_t DATA_SEGMENT_SIZE = 1024;      // 1 KB

TEST_CASE("Signals Size") {
	CHECK(sizeof(Signals) == 4);
}

TEST_CASE("ILOAD") {
	std::vector<uint8_t> program = { ILOAD_ADDR, 0x2, HALT_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN (dispatch -> ILOAD; queue front = index)
	mic.Cycle(); // ILOAD1 (MAR = LV + index, rd)
	mic.Cycle(); // ILOAD2 (SP+1)
	mic.Cycle(); // ILOAD3 (wr)
	mic.Cycle(); // ILOAD4 (TOS = MDR, fused dispatch -> HALT)
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[program[1]]);

	auto output = memory.GetData(data.size() + 1);
	CHECK(output[data.size()] == regs.tos);
}

TEST_CASE("WIDE_ILOAD") {
	std::vector<uint8_t> program = { WIDE_ADDR, ILOAD_ADDR, 0x0, 0x2,
		HALT_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN (dispatch -> WIDE)
	mic.Cycle(); // WIDE2 (dispatch on the widened opcode)
	mic.Cycle(); // WIDE_ILOAD1 (MAR = LV + 16-bit index, rd)
	mic.Cycle(); // WIDE_ILOAD2 (SP+1)
	mic.Cycle(); // WIDE_ILOAD3 (wr)
	mic.Cycle(); // WIDE_ILOAD4 (TOS = MDR, fused dispatch -> HALT)
	auto regs = mic.GetRegisters();

	auto varnum = (program[2] << 8) | program[3];
	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[varnum]);

	auto output = memory.GetData(data.size() + 1);
	CHECK(output[data.size()] == regs.tos);
}

TEST_CASE("IADD") {
	std::vector<uint8_t> program = { IADD_ADDR, HALT_ADDR };
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
	mic.Cycle(); // IADD1 (SP-1, rd)
	mic.Cycle(); // IADD2 (read-latency pad)
	mic.Cycle(); // IADD3 (fused dispatch -> HALT)
	mic.Cycle(); // HALT1
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1] + data[sp]);

	auto output = memory.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("ISUB") {
	std::vector<uint8_t> program = { ISUB_ADDR, HALT_ADDR };
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
	mic.Cycle(); // ISUB3 (fused dispatch -> HALT)
	mic.Cycle(); // HALT1
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1] - data[sp]);

	auto output = memory.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("IAND") {
	std::vector<uint8_t> program = { IAND_ADDR, HALT_ADDR };
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
	mic.Cycle(); // IAND1
	mic.Cycle(); // IAND2
	mic.Cycle(); // IAND3 (fused dispatch -> HALT)
	mic.Cycle(); // HALT1
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == (data[sp - 1] & data[sp]));

	auto output = memory.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("IOR") {
	std::vector<uint8_t> program = { IOR_ADDR, HALT_ADDR };
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
	mic.Cycle(); // IOR1
	mic.Cycle(); // IOR2
	mic.Cycle(); // IOR3 (fused dispatch -> HALT)
	mic.Cycle(); // HALT1
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
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
	mic.Cycle(); // DUP2 (fused dispatch -> padding byte 0 = NOP)
	mic.Cycle(); // NOP (delivers DUP2's pending write)
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size() + 1);

	auto output = memory.GetData(data.size() + 1);
	CHECK(output[sp + 1] == data[sp]);
}

TEST_CASE("POP") {
	std::vector<uint8_t> program = { POP_ADDR, HALT_ADDR };
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
	mic.Cycle(); // POP3 (fused dispatch -> HALT)
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
	mic.Cycle(); // SWAP6 (fused dispatch -> padding byte 0 = NOP)
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp);
	CHECK(regs.pc == program.size() + 1);
	CHECK(regs.tos == data[sp - 1]);

	auto output = memory.GetData(data.size());
	CHECK(output[sp] == data[sp - 1]);
	CHECK(output[sp - 1] == data[sp]);
}

TEST_CASE("ISTORE") {
	std::vector<uint8_t> program = { ISTORE_ADDR, 0x1, HALT_ADDR };
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
	mic.Cycle(); // ISTORE1 (MAR = LV + index)
	mic.Cycle(); // ISTORE2 (write TOS to the slot)
	mic.Cycle(); // ISTORE3 (SP-1, rd)
	mic.Cycle(); // ISTORE4 (read-latency pad)
	mic.Cycle(); // ISTORE5 (TOS = MDR, fused dispatch -> HALT)
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1]);

	auto output = memory.GetData(data.size());
	CHECK(output[program[1]] == data[sp]);
}

TEST_CASE("WIDE_ISTORE") {
	std::vector<uint8_t> program = { WIDE_ADDR, ISTORE_ADDR, 0x0, 0x1,
		HALT_ADDR };
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
	mic.Cycle(); // MAIN (dispatch -> WIDE)
	mic.Cycle(); // WIDE2 (dispatch on the widened opcode)
	mic.Cycle(); // WIDE_ISTORE1 (MAR = LV + 16-bit index)
	mic.Cycle(); // WIDE_ISTORE2 (write TOS to the slot)
	mic.Cycle(); // WIDE_ISTORE3 (SP-1, rd)
	mic.Cycle(); // WIDE_ISTORE4 (read-latency pad)
	mic.Cycle(); // WIDE_ISTORE5 (TOS = MDR, fused dispatch -> HALT)
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1]);

	auto varnum = (program[2] << 8) | program[3];
	auto output = memory.GetData(data.size());
	CHECK(output[varnum] == data[sp]);
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
	mic.Cycle(); // MAIN (queue front = the operand)
	mic.Cycle(); // BIPUSH1 (SP+1)
	mic.Cycle(); // BIPUSH2 (TOS = sign-extended operand, fused dispatch)
	mic.Cycle(); // NOP (dispatched; delivers BIPUSH2's pending write)
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
	mic.Cycle(); // IF_ICMPEQ1
	mic.Cycle(); // IF_ICMPEQ2
	mic.Cycle(); // IF_ICMPEQ3
	mic.Cycle(); // IF_ICMPEQ4
	mic.Cycle(); // IF_ICMPEQ5
	mic.Cycle(); // IF_ICMPEQ6 (jamz -> T)
	mic.Cycle(); // T (OPC = branch address)
	mic.Cycle(); // GOTO2 (write_pc: jump to the target)
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
	mic.Cycle(); // IF_ICMPEQ1
	mic.Cycle(); // IF_ICMPEQ2
	mic.Cycle(); // IF_ICMPEQ3
	mic.Cycle(); // IF_ICMPEQ4
	mic.Cycle(); // IF_ICMPEQ5
	mic.Cycle(); // IF_ICMPEQ6 (jamz -> F)
	mic.Cycle(); // F1 (discard the 2-byte offset)
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
	mic.Cycle(); // GOTO1 (OPC = goto's own address)
	mic.Cycle(); // GOTO2 (PC = OPC + 16-bit offset: jump)
	auto regs = mic.GetRegisters();

	CHECK(regs.pc == ((program[1] << 8) | program[2]));
}

TEST_CASE("LDC_W") {
	std::vector<uint8_t> program = { LDC_W_ADDR, 0x0, 0x1, HALT_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	std::vector<uint32_t> constants = { 7, 8 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);
	memory.SetConstants(constants);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN (queue front = 16-bit index)
	mic.Cycle(); // LDC_W1 (MAR = CPP + index, rd)
	mic.Cycle(); // LDC_W2 (SP+1)
	mic.Cycle(); // LDC_W3 (TOS = MDR, wr, fused dispatch -> HALT)
	auto regs = mic.GetRegisters();

	auto index = ((program[1] << 8) | program[2]);
	CHECK(regs.pc == program.size());
	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.tos == constants[index]);
}

TEST_CASE("IINC") {
	std::vector<uint8_t> program = { IINC_ADDR, 0x1, 0x2, HALT_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN (queue front = index)
	mic.Cycle(); // IINC1 (MAR = LV + index, rd)
	mic.Cycle(); // IINC2 (read-latency pad)
	mic.Cycle(); // IINC3 (MDR = MDR + const, wr, fused dispatch -> HALT)
	mic.Cycle(); // HALT1 (delivers IINC3's pending write)
	auto regs = mic.GetRegisters();

	CHECK(regs.pc == program.size());

	auto output = memory.GetData(data.size());
	CHECK(output[program[1]] == data[program[1]] + program[2]);
}

TEST_CASE("IFLT Less") {
	std::vector<uint8_t> program = { IFLT_ADDR, 0x1, 0x2, GOTO_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, static_cast<uint32_t>(-50) };
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
	mic.Cycle(); // IFLT1
	mic.Cycle(); // IFLT2
	mic.Cycle(); // IFLT3
	mic.Cycle(); // IFLT4 (jamn -> T)
	mic.Cycle(); // T (OPC = branch address)
	mic.Cycle(); // GOTO2 (jump)
	auto regs = mic.GetRegisters();

	auto offset = ((program[1] << 8) | program[2]);
	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == offset);
	CHECK(regs.tos == data[sp - 1]);
}

TEST_CASE("IFLT More") {
	std::vector<uint8_t> program = { IFLT_ADDR, 0x1, 0x2, GOTO_ADDR };
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
	mic.Cycle(); // IFLT1
	mic.Cycle(); // IFLT2
	mic.Cycle(); // IFLT3
	mic.Cycle(); // IFLT4 (jamn -> F)
	mic.Cycle(); // F1 (discard the 2-byte offset)
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == 3);
	CHECK(regs.mbr == GOTO_ADDR);
	CHECK(regs.tos == data[sp - 1]);
}

TEST_CASE("IFLT Equal") {
	std::vector<uint8_t> program = { IFLT_ADDR, 0x1, 0x2, GOTO_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 0 };
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
	mic.Cycle(); // IFLT1
	mic.Cycle(); // IFLT2
	mic.Cycle(); // IFLT3
	mic.Cycle(); // IFLT4 (jamn -> F)
	mic.Cycle(); // F1 (discard the 2-byte offset)
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == 3);
	CHECK(regs.mbr == GOTO_ADDR);
	CHECK(regs.tos == data[sp - 1]);
}

TEST_CASE("IFEQ Equal") {
	std::vector<uint8_t> program = { IFEQ_ADDR, 0x1, 0x2, GOTO_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 0 };
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
	mic.Cycle(); // IFEQ1
	mic.Cycle(); // IFEQ2
	mic.Cycle(); // IFEQ3
	mic.Cycle(); // IFEQ4 (jamz -> T)
	mic.Cycle(); // T
	mic.Cycle(); // GOTO2 (jump)
	auto regs = mic.GetRegisters();

	auto offset = ((program[1] << 8) | program[2]);
	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == offset);
	CHECK(regs.tos == data[sp - 1]);
}

TEST_CASE("IFEQ Nonequal") {
	std::vector<uint8_t> program = { IFEQ_ADDR, 0x1, 0x2, GOTO_ADDR };
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
	mic.Cycle(); // IFEQ1
	mic.Cycle(); // IFEQ2
	mic.Cycle(); // IFEQ3
	mic.Cycle(); // IFEQ4 (jamz -> F)
	mic.Cycle(); // F1 (discard the 2-byte offset)
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == 3);
	CHECK(regs.mbr == GOTO_ADDR);
	CHECK(regs.tos == data[sp - 1]);
}

TEST_CASE("INVOKEVIRTUAL") {
	constexpr uint8_t params_count = 0x3;
	constexpr uint8_t local_vars_count = 0x4;
	std::vector<uint8_t> program = {
		INVOKEVIRTUAL_ADDR, 0x0, 0x1, 0x0,
		/* new procedure*/
		/* params count (2 bytes) */ 0x0, params_count,
		/* local variables size (2 bytes) */ 0x0, local_vars_count,
	    /* first instruction */ BIPUSH_ADDR };
	std::vector<uint32_t> data = {
		/* main procedure */
		0, 10, 20, 30,
		/* new procedure call */
		/* OBJREF */ 0, /* param1 */ 70, /* param2 */ 80,
		/* local vars */ };
	std::vector<uint32_t> constants = { 0x0, 0x4 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);
	memory.SetConstants(constants);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN (queue front = 16-bit index)
	mic.Cycle(); // INVOKEVIRTUAL1 (H = index)
	mic.Cycle(); // INVOKEVIRTUAL2 (MAR = CPP + index, rd)
	mic.Cycle(); // INVOKEVIRTUAL3 (OPC = return address)
	mic.Cycle(); // INVOKEVIRTUAL4 (PC = callee address: jump)
	mic.Cycle(); // stall (callee block in flight)
	mic.Cycle(); // INVOKEVIRTUAL5 (H = params_hi << 8)
	mic.Cycle(); // INVOKEVIRTUAL6 (H |= params_lo)
	mic.Cycle(); // INVOKEVIRTUAL7 (TOS = SP - params)
	mic.Cycle(); // INVOKEVIRTUAL8 (TOS+1, MAR)
	mic.Cycle(); // INVOKEVIRTUAL9 (H = locals_hi << 8)
	mic.Cycle(); // INVOKEVIRTUAL10 (H |= locals_lo)
	mic.Cycle(); // INVOKEVIRTUAL11 (MDR = SP + locals + 1, wr)
	mic.Cycle(); // INVOKEVIRTUAL12 (MAR, SP = MDR)
	mic.Cycle(); // INVOKEVIRTUAL13 (MDR = OPC, wr)
	mic.Cycle(); // INVOKEVIRTUAL14 (SP+1, MAR)
	mic.Cycle(); // INVOKEVIRTUAL15 (MDR = LV, wr)
	mic.Cycle(); // INVOKEVIRTUAL16 (LV = TOS)

	auto output = memory.GetData(data.size() + local_vars_count + 1 /* pc */ + 1 /* lv */);
	auto objref = output[4];
	CHECK(objref == memory.GetDataSegmentAddress() + data.size() + local_vars_count);
	auto called_proc_pc = output[data.size() + local_vars_count];
	CHECK(called_proc_pc == 3);
	auto called_proc_lv = output[data.size() + local_vars_count + 1];
	CHECK(called_proc_lv == memory.GetDataSegmentAddress());

	auto regs = mic.GetRegisters();
	CHECK(regs.sp == memory.GetDataSegmentAddress() + data.size() + local_vars_count + 1);
	// PC = the callee's first opcode (the queue front), one past the
	// invokevirtual's own bytes.
	CHECK(regs.pc == 8);
	CHECK(regs.lv == memory.GetDataSegmentAddress() + 4);
	CHECK(regs.mbr == BIPUSH_ADDR);
}

TEST_CASE("IRETURN") {
	std::vector<uint8_t> program = { IRETURN_ADDR, 0, 0, BIPUSH_ADDR };
	std::vector<uint32_t> data = {
		/* main procedure */
		0, 10, 20, 30,
		/* new procedure call */
		/* OBJREF */ 0, /* param1 */ 70, /* param2 */ 80,
		/* local vars */ 0, 0, 0, 0,
	    /* called proc pc */ 3,
	    /* called proc lv */ 0,
	    /* return value */ 0x7 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	uint32_t objref = memory.GetDataSegmentAddress() + 11;
	data[4] = objref;
	uint32_t called_proc_lv = memory.GetDataSegmentAddress();
	data[12] = called_proc_lv;
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);
	mic.SetLV(memory.GetDataSegmentAddress() + 4);

	mic.Cycle(); // NOP
	mic.Cycle(); // MAIN
	mic.Cycle(); // IRETURN1
	mic.Cycle(); // IRETURN2
	mic.Cycle(); // IRETURN3
	mic.Cycle(); // IRETURN4
	mic.Cycle(); // IRETURN5 (PC = savedPC: jump + refill)
	mic.Cycle(); // IRETURN6
	mic.Cycle(); // IRETURN7
	mic.Cycle(); // IRETURN8 (fused dispatch: the queue front is already
	             // the return target's opcode)
	mic.Cycle(); // BIPUSH1 (delivers IRETURN8's pending write)

	auto output = memory.GetData(5);
	auto retval = output[output.size() - 1];
	CHECK(retval == data[13]);

	auto regs = mic.GetRegisters();
	// SP = frame base + 1: the return restored SP to the caller frame
	// and the dispatched bipush has already pushed its operand slot.
	CHECK(regs.sp == memory.GetDataSegmentAddress() + 5);
	// PC = one past the return target: the dispatch consumed its opcode.
	CHECK(regs.pc == 4);
	CHECK(regs.lv == memory.GetDataSegmentAddress());
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

TEST_CASE("Program Equal Wide") {
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
		WIDE_ADDR, ISTORE_ADDR, 0x01, 0x02,
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

TEST_CASE("ISHR") {
	// ishr: TOS = TOS >> 1 (arithmetic, sign-filled)
	std::vector<uint8_t> program = { ISHR_ADDR, HALT_ADDR };

	// positive: 8 >> 1 = 4
	{
		GlobalMemory memory(
			PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
		memory.SetProgram(program);

		MIC mic(&memory);
		mic.SetTOS(8);

		mic.Cycle(); // NOP
		mic.Cycle(); // MAIN
		mic.Cycle(); // ISHR (fused dispatch -> HALT)
		auto regs = mic.GetRegisters();
		CHECK(regs.tos == 4);
	}

	// negative: -4 >> 1 = -2 (0xFFFFFFFC >> 1 = 0xFFFFFFFE)
	{
		GlobalMemory memory(
			PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
		memory.SetProgram(program);

		MIC mic(&memory);
		mic.SetTOS(0xFFFFFFFCu);

		mic.Cycle(); // NOP
		mic.Cycle(); // MAIN
		mic.Cycle(); // ISHR
		auto regs = mic.GetRegisters();
		CHECK(regs.tos == 0xFFFFFFFEu);
	}

	// 1 >> 1 = 0, and 0 >> 1 = 0
	{
		GlobalMemory memory(
			PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
		memory.SetProgram(program);

		MIC mic(&memory);
		mic.SetTOS(1);

		mic.Cycle(); // NOP
		mic.Cycle(); // MAIN (queue front = HALT)
		CHECK(mic.GetRegisters().mbr == HALT_ADDR);
		mic.Cycle(); // ISHR
		CHECK(mic.GetRegisters().tos == 0);
	}
}
