#pragma once

#include "../micro_instruction.h"

// Reserved dispatch base for wide opcodes: WIDE2's JMPC dispatches to
// 0x100 | widened_opcode. The MAIN dispatch microinstruction itself no
// longer exists -- NOP (0x00) is the decode step, and the jump path uses
// GOTO3/GOTO4.
constexpr uint32_t MAIN_ADDR = 0x100;
