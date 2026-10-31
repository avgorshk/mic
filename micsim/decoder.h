#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#include "control_memory.h"
#include "ifu.h"

constexpr uint32_t WIDE_ADDR = 0xCA;

// The Instruction Decoder: an SRAM mapping every instruction opcode to
// its routine index and its full footprint in program bytes (opcode +
// operand bytes). It WALKS the byte stream -- it only peeks the IFU's
// queue at its walk offset (never consumes), advances by the footprint
// on each successful decode, and resyncs to the IFU's PC after every
// jump. WIDE is a two-step lookup into the wide table at
// 0x100 | inner opcode. An invalid opcode is an assert (debug).
class Decoder {
public:
	struct Entry {
		bool valid = false;
		uint8_t size = 0;   // full footprint: opcode + operand bytes
		uint8_t pop = 1;    // opcode bytes the routine's first MIR pops
		uint8_t index = 0;  // routine index into the control memory
	};

	Decoder() : table_(512) {
		Add(NOP_ADDR, 1, NOP_INDEX);
		Add(BIPUSH_ADDR, 2, BIPUSH_INDEX);
		Add(LDC_W_ADDR, 3, LDC_W_INDEX);
		Add(ILOAD_ADDR, 2, ILOAD_INDEX);
		Add(ISTORE_ADDR, 2, ISTORE_INDEX);
		Add(POP_ADDR, 1, POP_INDEX);
		Add(DUP_ADDR, 1, DUP_INDEX);
		Add(SWAP_ADDR, 1, SWAP_INDEX);
		Add(IADD_ADDR, 1, IADD_INDEX);
		Add(ISUB_ADDR, 1, ISUB_INDEX);
		Add(IAND_ADDR, 1, IAND_INDEX);
		Add(IOR_ADDR, 1, IOR_INDEX);
		Add(IINC_ADDR, 3, IINC_INDEX);
		Add(ISHR_ADDR, 1, ISHR_INDEX);
		Add(IFEQ_ADDR, 3, IFEQ_INDEX);
		Add(IFLT_ADDR, 3, IFLT_INDEX);
		Add(IF_ICMPEQ_ADDR, 3, IF_ICMPEQ_INDEX);
		Add(GOTO_ADDR, 3, GOTO_INDEX);
		Add(IRETURN_ADDR, 1, IRETURN_INDEX);
		Add(INVOKEVIRTUAL_ADDR, 3, INVOKEVIRTUAL_INDEX);
		Add(HALT_ADDR, 1, HALT_INDEX);
		// WIDE itself is not an entry: it two-steps into the wide
		// table (0x100 | inner opcode), whose entries pop 2 bytes.
		Add(WIDE_ILOAD_ADDR, 4, WIDE_ILOAD_INDEX);
		Add(WIDE_ISTORE_ADDR, 4, WIDE_ISTORE_INDEX);
	}

	// Synchronous decode at the walk position. Returns false when the
	// needed bytes are not fetched yet -- the queue retries next cycle.
	// On success the walk advances by the footprint and the caller gets
	// the routine index plus the opcode-byte pop count. The walk stops
	// for good after a HALT: the program ends there, and the machine
	// halts at the HALT MIR's retire before the queue's confirm can
	// stall on it (the old in-order dispatch never looked past the
	// halt either).
	bool Decode(const InstructionFetchUnit& ifu, uint32_t& index,
	            uint8_t& pop) {
		if (halted_) {
			return false;
		}
		assert(walk_ >= ifu.GetPC());
		uint32_t off = walk_ - ifu.GetPC();
		if (off >= ifu.Available()) {
			return false;
		}
		uint8_t op = ifu.PeekAt(off);
		Entry e;
		if (op == WIDE_ADDR) {
			if (off + 1 >= ifu.Available()) {
				return false;
			}
			e = table_[0x100 | ifu.PeekAt(off + 1)];
		} else {
			e = table_[op];
		}
		if (!e.valid) {  // invalid opcode
			std::fprintf(stderr,
				"[decoder] invalid opcode %02X at walk=%u pc=%u "
				"off=%u\n",
				op, walk_, ifu.GetPC(), off);
			assert(0);
		}
		walk_ += e.size;
		index = e.index;
		pop = e.pop;
		last_index_ = e.index;
		if (index == HALT_INDEX) {
			halted_ = true;
		}
		return true;
	}

	// After a jump lands, the walk restarts at the jump target; any
	// wrong-path decode result is discarded. An invokevirtual jumps to
	// the callee's 4-byte header (params + locals), not to its first
	// instruction -- the IJVM method header is always 4 bytes, so the
	// walk skips it when the landed jump belongs to the invokevirtual
	// that was decoded last (its confirm is held until the landing, so
	// nothing was decoded after it).
	void Resync(uint32_t addr) {
		walk_ = addr
			+ (last_index_ == INVOKEVIRTUAL_INDEX ? 4 : 0);
	}

	uint32_t GetWalk() const {
		return walk_;
	}

private:
	void Add(uint32_t opcode, uint8_t size, uint8_t index) {
		table_[opcode] = { true, size,
			(uint8_t)(opcode >= 0x100 ? 2 : 1), index };
	}

	std::vector<Entry> table_;
	uint32_t walk_ = 0;
	uint32_t last_index_ = 0;
	bool halted_ = false;
};
