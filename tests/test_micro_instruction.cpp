#include "doctest.h"

#include "micro_instruction.h"

TEST_CASE("MicroInstructionEncoding Size") {
	CHECK(sizeof(MicroInstructionEncoding) == 8);
}