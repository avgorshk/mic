#include "doctest.h"

#include "mic.h"

TEST_CASE("Signals Size") {
	CHECK(sizeof(Signals) == 4);
}