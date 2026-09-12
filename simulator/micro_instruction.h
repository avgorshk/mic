#pragma once

#include "alu.h"
#include "signals.h"

class MicroInstruction {
public:
	MicroInstruction(Signals encoding) : encoding_(encoding) {}

    

private:
	Signals encoding_ = { 0 };
};