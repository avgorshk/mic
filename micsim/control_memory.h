#pragma once

#include <cassert>
#include <vector>

#include "micro_instructions/micro_instructions.h"

// The Instruction Queue's MIR SRAM: every routine laid out contiguously
// (no cross-routine jumps; wide tails duplicated; each conditional
// branch ends with a dedicated branch MIR followed by the not-taken F1
// continuation). The queue copies a routine from its index until the
// end bit; the shared taken block is hardwired at TAKEN_BLOCK_INDEX.
constexpr size_t CONTROL_MEMORY_SIZE = 128;

constexpr uint32_t NOP_INDEX = 0;
constexpr uint32_t BIPUSH_INDEX = 1;
constexpr uint32_t LDC_W_INDEX = 3;
constexpr uint32_t ILOAD_INDEX = 6;
constexpr uint32_t ISTORE_INDEX = 10;
constexpr uint32_t POP_INDEX = 15;
constexpr uint32_t DUP_INDEX = 18;
constexpr uint32_t SWAP_INDEX = 20;
constexpr uint32_t IADD_INDEX = 26;
constexpr uint32_t ISUB_INDEX = 29;
constexpr uint32_t IAND_INDEX = 32;
constexpr uint32_t IOR_INDEX = 35;
constexpr uint32_t IINC_INDEX = 38;
constexpr uint32_t ISHR_INDEX = 41;
constexpr uint32_t HALT_INDEX = 42;
constexpr uint32_t WIDE_ILOAD_INDEX = 43;
constexpr uint32_t WIDE_ISTORE_INDEX = 47;
constexpr uint32_t GOTO_INDEX = 52;
constexpr uint32_t IRETURN_INDEX = 55;
constexpr uint32_t INVOKEVIRTUAL_INDEX = 63;
constexpr uint32_t IFEQ_INDEX = 79;        // + branch MIR + F1
constexpr uint32_t IFLT_INDEX = 85;        // + branch MIR + F1
constexpr uint32_t IF_ICMPEQ_INDEX = 91;   // + branch MIR + F1
constexpr uint32_t TAKEN_BLOCK_INDEX = 99; // T, GOTO2, GOTO3

class ControlMemory {
public:
	ControlMemory() : memory_(CONTROL_MEMORY_SIZE) {
		memory_[NOP_INDEX] = NOPMicroInstruction();

		memory_[BIPUSH_INDEX] = BIPUSH1MicroInstruction();
		memory_[BIPUSH_INDEX + 1] = BIPUSH2MicroInstruction();

		memory_[LDC_W_INDEX] = LDC_W1MicroInstruction();
		memory_[LDC_W_INDEX + 1] = LDC_W2MicroInstruction();
		memory_[LDC_W_INDEX + 2] = LDC_W3MicroInstruction();

		memory_[ILOAD_INDEX] = ILOAD1MicroInstruction();
		memory_[ILOAD_INDEX + 1] = ILOAD2MicroInstruction();
		memory_[ILOAD_INDEX + 2] = ILOAD3MicroInstruction();
		memory_[ILOAD_INDEX + 3] = ILOAD4MicroInstruction();

		memory_[ISTORE_INDEX] = ISTORE1MicroInstruction();
		memory_[ISTORE_INDEX + 1] = ISTORE2MicroInstruction();
		memory_[ISTORE_INDEX + 2] = ISTORE3MicroInstruction();
		memory_[ISTORE_INDEX + 3] = ISTORE4MicroInstruction();
		memory_[ISTORE_INDEX + 4] = ISTORE5MicroInstruction();

		memory_[POP_INDEX] = POP1MicroInstruction();
		memory_[POP_INDEX + 1] = POP2MicroInstruction();
		memory_[POP_INDEX + 2] = POP3MicroInstruction();

		memory_[DUP_INDEX] = DUP1MicroInstruction();
		memory_[DUP_INDEX + 1] = DUP2MicroInstruction();

		memory_[SWAP_INDEX] = SWAP1MicroInstruction();
		memory_[SWAP_INDEX + 1] = SWAP2MicroInstruction();
		memory_[SWAP_INDEX + 2] = SWAP3MicroInstruction();
		memory_[SWAP_INDEX + 3] = SWAP4MicroInstruction();
		memory_[SWAP_INDEX + 4] = SWAP5MicroInstruction();
		memory_[SWAP_INDEX + 5] = SWAP6MicroInstruction();

		memory_[IADD_INDEX] = IADD1MicroInstruction();
		memory_[IADD_INDEX + 1] = IADD2MicroInstruction();
		memory_[IADD_INDEX + 2] = IADD3MicroInstruction();

		memory_[ISUB_INDEX] = ISUB1MicroInstruction();
		memory_[ISUB_INDEX + 1] = ISUB2MicroInstruction();
		memory_[ISUB_INDEX + 2] = ISUB3MicroInstruction();

		memory_[IAND_INDEX] = IAND1MicroInstruction();
		memory_[IAND_INDEX + 1] = IAND2MicroInstruction();
		memory_[IAND_INDEX + 2] = IAND3MicroInstruction();

		memory_[IOR_INDEX] = IOR1MicroInstruction();
		memory_[IOR_INDEX + 1] = IOR2MicroInstruction();
		memory_[IOR_INDEX + 2] = IOR3MicroInstruction();

		memory_[IINC_INDEX] = IINC1MicroInstruction();
		memory_[IINC_INDEX + 1] = IINC2MicroInstruction();
		memory_[IINC_INDEX + 2] = IINC3MicroInstruction();

		memory_[ISHR_INDEX] = ISHRMicroInstruction();

		memory_[HALT_INDEX] = HALTMicroInstruction();

		memory_[WIDE_ILOAD_INDEX] = WIDE_ILOAD1MicroInstruction();
		memory_[WIDE_ILOAD_INDEX + 1] = WIDE_ILOAD2MicroInstruction();
		memory_[WIDE_ILOAD_INDEX + 2] = WIDE_ILOAD3MicroInstruction();
		memory_[WIDE_ILOAD_INDEX + 3] = WIDE_ILOAD4MicroInstruction();

		memory_[WIDE_ISTORE_INDEX] = WIDE_ISTORE1MicroInstruction();
		memory_[WIDE_ISTORE_INDEX + 1] = WIDE_ISTORE2MicroInstruction();
		memory_[WIDE_ISTORE_INDEX + 2] = WIDE_ISTORE3MicroInstruction();
		memory_[WIDE_ISTORE_INDEX + 3] = WIDE_ISTORE4MicroInstruction();
		memory_[WIDE_ISTORE_INDEX + 4] = WIDE_ISTORE5MicroInstruction();

		memory_[GOTO_INDEX] = GOTO1MicroInstruction();
		memory_[GOTO_INDEX + 1] = GOTO2MicroInstruction();
		memory_[GOTO_INDEX + 2] = GOTO3MicroInstruction();

		memory_[IRETURN_INDEX] = IRETURN1MicroInstruction();
		memory_[IRETURN_INDEX + 1] = IRETURN2MicroInstruction();
		memory_[IRETURN_INDEX + 2] = IRETURN3MicroInstruction();
		memory_[IRETURN_INDEX + 3] = IRETURN4MicroInstruction();
		memory_[IRETURN_INDEX + 4] = IRETURN5MicroInstruction();
		memory_[IRETURN_INDEX + 5] = IRETURN6MicroInstruction();
		memory_[IRETURN_INDEX + 6] = IRETURN7MicroInstruction();
		memory_[IRETURN_INDEX + 7] = IRETURN8MicroInstruction();

		memory_[INVOKEVIRTUAL_INDEX] = INVOKEVIRTUAL1MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 1] = INVOKEVIRTUAL2MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 2] = INVOKEVIRTUAL3MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 3] = INVOKEVIRTUAL4MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 4] = INVOKEVIRTUAL5MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 5] = INVOKEVIRTUAL6MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 6] = INVOKEVIRTUAL7MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 7] = INVOKEVIRTUAL8MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 8] = INVOKEVIRTUAL9MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 9] = INVOKEVIRTUAL10MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 10] = INVOKEVIRTUAL11MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 11] = INVOKEVIRTUAL12MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 12] = INVOKEVIRTUAL13MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 13] = INVOKEVIRTUAL14MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 14] = INVOKEVIRTUAL15MicroInstruction();
		memory_[INVOKEVIRTUAL_INDEX + 15] = INVOKEVIRTUAL16MicroInstruction();

		// ifeq: work MIRs + the dedicated branch MIR + the not-taken
		// F1 continuation.
		memory_[IFEQ_INDEX] = IFEQ1MicroInstruction();
		memory_[IFEQ_INDEX + 1] = IFEQ2MicroInstruction();
		memory_[IFEQ_INDEX + 2] = IFEQ3MicroInstruction();
		memory_[IFEQ_INDEX + 3] = IFEQ4MicroInstruction();
		memory_[IFEQ_INDEX + 4] = BranchZMicroInstruction();
		memory_[IFEQ_INDEX + 5] = F1MicroInstruction();

		memory_[IFLT_INDEX] = IFLT1MicroInstruction();
		memory_[IFLT_INDEX + 1] = IFLT2MicroInstruction();
		memory_[IFLT_INDEX + 2] = IFLT3MicroInstruction();
		memory_[IFLT_INDEX + 3] = IFLT4MicroInstruction();
		memory_[IFLT_INDEX + 4] = BranchNMicroInstruction();
		memory_[IFLT_INDEX + 5] = F1MicroInstruction();

		memory_[IF_ICMPEQ_INDEX] = IF_ICMPEQ1MicroInstruction();
		memory_[IF_ICMPEQ_INDEX + 1] = IF_ICMPEQ2MicroInstruction();
		memory_[IF_ICMPEQ_INDEX + 2] = IF_ICMPEQ3MicroInstruction();
		memory_[IF_ICMPEQ_INDEX + 3] = IF_ICMPEQ4MicroInstruction();
		memory_[IF_ICMPEQ_INDEX + 4] = IF_ICMPEQ5MicroInstruction();
		memory_[IF_ICMPEQ_INDEX + 5] = IF_ICMPEQ6MicroInstruction();
		memory_[IF_ICMPEQ_INDEX + 6] = BranchZMicroInstruction();
		memory_[IF_ICMPEQ_INDEX + 7] = F1MicroInstruction();

		// The shared taken block: T (OPC = the branch's own address),
		// GOTO2 (consume the offset, write the PC), GOTO3 (the
		// scheduled refill wait + end bit).
		memory_[TAKEN_BLOCK_INDEX] = TMicroInstruction();
		memory_[TAKEN_BLOCK_INDEX + 1] = GOTO2MicroInstruction();
		memory_[TAKEN_BLOCK_INDEX + 2] = GOTO3MicroInstruction();
	}

public:
	MicroInstruction FetchMIR(uint32_t addr) const {
		assert(addr < CONTROL_MEMORY_SIZE);
		return memory_[addr];
	}

private:
	std::vector<MicroInstruction> memory_;
};
