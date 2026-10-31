#include "doctest.h"

#include "global_memory.h"
#include "mic.h"
#include "micro_instructions/micro_instructions.h"

constexpr uint32_t PROGRAM_SEGMENT_SIZE = 1024;   // 1 KB
constexpr uint32_t CONSTANT_SEGMENT_SIZE = 1024;  // 1 KB
constexpr uint32_t DATA_SEGMENT_SIZE = 1024;      // 1 KB

// The pipelined machine (4-slot conveyor): tests either run to halt or
// run a fixed number of cycles. Startup: NOP dispatches the first
// opcode at cycle 3 (flush +2), the first routine MIR enters S1 at
// cycle 4; a routine MIR retires at fetch cycle + 2.
static void RunToHalt(MIC& mic) {
	int guard = 0;
	while (!mic.Cycle() && ++guard < 1000) {
	}
	CHECK(guard < 1000);
}

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

	RunToHalt(mic);
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
	RunToHalt(mic);
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

	RunToHalt(mic);
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

	RunToHalt(mic);
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

	RunToHalt(mic);
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

	RunToHalt(mic);
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == (data[sp - 1] | data[sp]));

	auto output = memory.GetData(data.size());
	CHECK(output[sp - 1] == regs.tos);
}

TEST_CASE("DUP") {
	std::vector<uint8_t> program = { DUP_ADDR, HALT_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	RunToHalt(mic);
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size());

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

	RunToHalt(mic);
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1]);
}

TEST_CASE("SWAP") {
	std::vector<uint8_t> program = { SWAP_ADDR, HALT_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30, 40, 50 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);
	mic.SetTOS(data[sp]);

	RunToHalt(mic);
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp);
	CHECK(regs.pc == program.size());
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

	RunToHalt(mic);
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

	RunToHalt(mic);
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp - 1);
	CHECK(regs.pc == program.size());
	CHECK(regs.tos == data[sp - 1]);

	auto varnum = (program[2] << 8) | program[3];
	auto output = memory.GetData(data.size());
	CHECK(output[varnum] == data[sp]);
}

TEST_CASE("BIPUSH") {
	std::vector<uint8_t> program = { BIPUSH_ADDR, 0x7, HALT_ADDR };
	std::vector<uint32_t> data = { 0, 10, 20, 30 };
	uint32_t sp = static_cast<uint32_t>(data.size()) - 1;

	GlobalMemory memory(
		PROGRAM_SEGMENT_SIZE, CONSTANT_SEGMENT_SIZE, DATA_SEGMENT_SIZE);
	memory.SetProgram(program);
	memory.SetData(data);

	MIC mic(&memory);
	mic.SetSP(memory.GetDataSegmentAddress() + sp);

	RunToHalt(mic);
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == regs.lv + sp + 1);
	CHECK(regs.pc == program.size());
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

	// T (taken path) -> GOTO2 lands the jump at cycle 19.
	for (int i = 0; i < 19; ++i) {
		mic.Cycle();
	}
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

	// F1 discards the offset at cycle 15.
	for (int i = 0; i < 16; ++i) {
		mic.Cycle();
	}
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

	// GOTO2's write_pc lands the jump at cycle 7.
	for (int i = 0; i < 7; ++i) {
		mic.Cycle();
	}
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

	RunToHalt(mic);
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

	RunToHalt(mic);
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

	// Taken: IFLT4 (jamn) -> T -> GOTO2 lands the jump at cycle 15.
	for (int i = 0; i < 15; ++i) {
		mic.Cycle();
	}
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

	// Not-taken: F1 discards the offset at cycle 11.
	for (int i = 0; i < 12; ++i) {
		mic.Cycle();
	}
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

	for (int i = 0; i < 12; ++i) {
		mic.Cycle();
	}
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

	// Taken: IFEQ4 (jamz) -> T -> GOTO2 lands the jump at cycle 15.
	for (int i = 0; i < 15; ++i) {
		mic.Cycle();
	}
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

	for (int i = 0; i < 12; ++i) {
		mic.Cycle();
	}
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

	// IV16's dispatch (LV = TOS + jmpc) resolves at cycle 26.
	for (int i = 0; i < 26; ++i) {
		mic.Cycle();
	}

	auto output = memory.GetData(data.size() + local_vars_count + 1 /* pc */ + 1 /* lv */);
	auto objref = output[4];
	CHECK(objref == memory.GetDataSegmentAddress() + data.size() + local_vars_count);
	auto called_proc_pc = output[data.size() + local_vars_count];
	CHECK(called_proc_pc == 3);
	auto called_proc_lv = output[data.size() + local_vars_count + 1];
	CHECK(called_proc_lv == memory.GetDataSegmentAddress());

	auto regs = mic.GetRegisters();
	CHECK(regs.sp == memory.GetDataSegmentAddress() + data.size() + local_vars_count + 1);
	// PC = one past the callee's first opcode: IV16's dispatch consumed it.
	CHECK(regs.pc == 9);
	CHECK(regs.lv == memory.GetDataSegmentAddress() + 4);
	CHECK(regs.mbr == 0x0);
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

	// IRETURN8's dispatch resolves at cycle 19; its pending write and
	// the return target's bipush land by cycle 22.
	for (int i = 0; i < 22; ++i) {
		mic.Cycle();
	}

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

		RunToHalt(mic);
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

		RunToHalt(mic);
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

		// The startup NOP dispatch lands at cycle 3: the queue front is
		// then the HALT opcode.
		for (int i = 0; i < 3; ++i) {
			mic.Cycle();
		}
		CHECK(mic.GetRegisters().mbr == HALT_ADDR);
		while (!mic.Cycle()) {
		}
		CHECK(mic.GetRegisters().tos == 0);
	}
}
