#include "doctest.h"

#include "alu.h"

class ALUTestObject {
public:
	static ALU& Create() {
		static ALU alu;
		alu.Reset();
		return alu;
	}
};

TEST_CASE("ALUFunction Size") {
	CHECK(sizeof(ALUFunction) == 1);
}

TEST_CASE("ALUFunction AND") {
	auto& alu = ALUTestObject::Create();
	alu.SetInput(4, 5);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::AND, 1, 1, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 4);
}

TEST_CASE("ALUFunction OR") {
	auto& alu = ALUTestObject::Create();
	alu.SetInput(4, 5);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::OR, 1, 0, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 4);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::OR, 0, 1, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 5);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::OR, 1, 0, 1, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == ~4);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::OR, 1, 1, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 5);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::OR, 0, 0, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 0);
}

TEST_CASE("ALUFunction INVB") {
	auto& alu = ALUTestObject::Create();
	alu.SetInput(4, 5);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::INVB, 0, 1, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == ~5);
}

TEST_CASE("ALUFunction SUM") {
	auto& alu = ALUTestObject::Create();
	alu.SetInput(4, 5);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 9);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 0, 1, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 10);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 0, 0, 1, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 5);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 0, 1, 0, 1, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 6);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 1, 1, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 1);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 0, 1, 1, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 4);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 0, 1, 1, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == -4);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 0, 0, 0, 1, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 1);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 0, 0, 1, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == -1);
}

TEST_CASE("ALUFunction SLL8") {
	auto& alu = ALUTestObject::Create();
	alu.SetInput(4, 5);
	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 0, 0, 1, 0 });
	alu.Execute();
	alu.Shift();
	CHECK(alu.GetResult() == (9 << 8));
}

TEST_CASE("ALUFunction SRA1") {
	auto& alu = ALUTestObject::Create();
	alu.SetInput(4, 5);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 0, 0, 0, 1 });
	alu.Execute();
	alu.Shift();
	CHECK(alu.GetResult() == (9 >> 1));

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 0, 1, 1, 0, 1 });
	alu.Execute();
	alu.Shift();
	CHECK(alu.GetResult() == -2);
}

TEST_CASE("ALUFunction Z") {
	auto& alu = ALUTestObject::Create();

	alu.SetInput(4, 4);
	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 8);
	CHECK(alu.GetZ() == 0);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 1, 1, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 0);
	CHECK(alu.GetZ() == 1);
}

TEST_CASE("ALUFunction N") {
	auto& alu = ALUTestObject::Create();

	alu.SetInput(5, 4);
	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 0, 0, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == 9);
	CHECK(alu.GetN() == 0);

	alu.SetFunction(ALUFunction{ ALUFunctionTypes::SUM, 1, 1, 1, 1, 0, 0 });
	alu.Execute();
	CHECK(alu.GetResult() == -1);
	CHECK(alu.GetN() == 1);
}