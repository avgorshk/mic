#include "doctest.h"

#include "mic.h"

TEST_CASE("Signals Size") {
	CHECK(sizeof(Signals) == 4);
}

TEST_CASE("Increment SP") {
	MIC mic;

	ALUFunction func = { 0 };
	func.func = ALUFunctionTypes::SUM;
	func.enb = 1;
	func.inc = 1;

	MicroInstruction inst;
	inst.alu = func;
	inst.read = 4;
	inst.write_sp = 1;

	mic.SetMicroInstruction(inst);
	mic.Cycle();
	auto regs = mic.GetRegisters();

	CHECK(regs.sp == 1);
}


TEST_CASE("Signed MBR To H") {
	MIC mic;

	ALUFunction func = { 0 };
	func.func = ALUFunctionTypes::SUM;
	func.enb = 1;
	func.inc = 1;

	MicroInstruction inst;
	inst.alu = func;
	inst.read = 3;
	inst.write_h = 1;

	mic.SetMicroInstruction(inst);
	mic.Cycle();
	auto regs = mic.GetRegisters();

	CHECK(regs.h == 1);
}