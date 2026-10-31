#pragma once

#include <assert.h>
#include <cstring>
#include <deque>
#include <stdint.h>

#include "global_memory.h"

// Instruction Fetch Unit: owns the PC, the prefetch address (IMAR) and a
// 6-byte shift-register queue of program bytes. Refills 4 bytes at a time
// from memory (one outstanding refill, 1-cycle delivery latency, IMAR
// auto-increments by 4 -- the ALU is never involved). MBR1 is the queue's
// front byte, MBR2 the two front bytes as a big-endian 16-bit word; reads
// via the ALU A-bus are the consumption events (Consume advances the
// queue and the PC). Jump flushes the queue and refills from the target.
class InstructionFetchUnit {
public:
	InstructionFetchUnit()
		: memory_(nullptr) {}

	void Init(GlobalMemory* memory, uint32_t program_start) {
		memory_ = memory;
		pc_ = program_start;
		imar_ = program_start;
		// Pre-fill the first block so MAIN does not stall at startup.
		uint8_t is_written = 0;
		uint32_t block = 0;
		memory_->FetchBlock(1, imar_, is_written, block);
		imar_ += 4;
		memory_->FetchBlock(0, imar_, is_written, block);
		if (is_written) {
			PushBlock(block);
		}
	}

	uint32_t GetPC() const { return pc_; }

	uint8_t Read1() const {
		return queue_.empty() ? 0 : queue_.front();
	}

	uint16_t Read2() const {
		if (queue_.size() < 2) return 0;
		return (queue_[0] << 8) | queue_[1];
	}

	int32_t Read1Signed() const {
		uint8_t b = Read1();
		return (b & 0x80) ? (int32_t)(b | 0xFFFFFF00) : (int32_t)b;
	}

	int32_t Read2Signed() const {
		uint16_t w = Read2();
		return (w & 0x8000) ? (int32_t)(w | 0xFFFF0000) : (int32_t)w;
	}

	// Read-only peeks at an offset from the queue front (no consume).
	// The pipelined machine's S1 latch reads the operand bytes that sit
	// behind the older occupants' yet-unconsumed bytes.
	uint8_t PeekAt(uint32_t n) const {
		if (n >= queue_.size()) return 0;
		return queue_[n];
	}

	int32_t PeekAtSigned(uint32_t n) const {
		uint8_t b = PeekAt(n);
		return (b & 0x80) ? (int32_t)(b | 0xFFFFFF00) : (int32_t)b;
	}

	uint16_t Peek2At(uint32_t n) const {
		if (n + 1 >= queue_.size()) return 0;
		return (uint16_t)((queue_[n] << 8) | queue_[n + 1]);
	}

	int32_t Peek2AtSigned(uint32_t n) const {
		uint16_t w = Peek2At(n);
		return (w & 0x8000) ? (int32_t)(w | 0xFFFF0000) : (int32_t)w;
	}

	uint32_t Available() const {
		return static_cast<uint32_t>(queue_.size());
	}

	// True when the queue cannot satisfy this cycle's byte needs -- the
	// machine holds (no register writes, no MPC update) until the refill
	// lands.
	bool Stall(uint32_t need) const {
		return Available() < need;
	}

	// Advance the queue by n consumed bytes (1 or 2); PC follows the
	// queue front. The stall check guarantees enough bytes are available.
	void Consume(uint32_t n) {
		assert(n <= queue_.size());
		for (uint32_t i = 0; i < n; ++i) {
			queue_.pop_front();
		}
		pc_ += n;
	}

	// Program-counter change (branch / invokevirtual / ireturn): flush
	// the queue and refill from the target address. The refill itself is
	// issued by the trigger in Cycle() (queue below the threshold), so
	// its delivery lands one full cycle later -- the honest memory
	// latency. A stale in-flight refill is drained and discarded.
	void Jump(uint32_t addr) {
		queue_.clear();
		pc_ = addr;
		imar_ = addr;
		if (refill_pending_) {
			uint8_t is_written = 0;
			uint32_t block = 0;
			memory_->FetchBlock(0, imar_, is_written, block);
			refill_pending_ = false;
		}
	}

	// Refill state machine: one call per machine cycle (including stall
	// cycles, so deliveries land while the datapath is held).
	void Cycle() {
		uint8_t is_written = 0;
		uint32_t block = 0;
		if (refill_pending_) {
			memory_->FetchBlock(0, imar_, is_written, block);
			if (is_written) {
				PushBlock(block);
				refill_pending_ = false;
			}
		}
		if (!refill_pending_ && queue_.size() <= 2) {
			IssueRefill();
		}
	}

private:
	void IssueRefill() {
		if (refill_pending_) return;
		uint8_t is_written = 0;
		uint32_t block = 0;
		memory_->FetchBlock(1, imar_, is_written, block);
		refill_pending_ = true;
		imar_ += 4;
	}

	void PushBlock(uint32_t block) {
		assert(queue_.size() + 4 <= 6);
		// Little-endian host: byte i of the block is (block >> 8*i).
		for (int i = 0; i < 4; ++i) {
			queue_.push_back((block >> (8 * i)) & 0xFF);
		}
	}

	GlobalMemory* memory_;
	std::deque<uint8_t> queue_;
	uint32_t pc_ = 0;
	uint32_t imar_ = 0;
	bool refill_pending_ = false;
};
