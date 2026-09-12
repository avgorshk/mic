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

	Signals signals = { 0 };
	signals.alu = func;
	signals.read_sp = 1;
	signals.write_sp = 1;

	mic.SetSignals(signals);
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

	Signals signals = { 0 };
	signals.alu = func;
	signals.read_mbr_signed = 1;
	signals.write_h = 1;

	Registers regs = { 0 };
	regs.mbr = static_cast<uint32_t>(-7);

	mic.SetSignals(signals);
	mic.SetRegisters(regs);
	mic.Cycle();
	regs = mic.GetRegisters();

	CHECK(regs.h == -6);
}