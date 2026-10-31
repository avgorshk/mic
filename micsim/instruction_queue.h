#pragma once

#include <assert.h>
#include <deque>
#include <stdint.h>

#include "control_memory.h"
#include "decoder.h"
#include "micro_instruction.h"
#include "signals.h"

constexpr size_t QUEUE_DEPTH = 16;

// The Instruction Queue: expands decoder routine indices into a FIFO of
// MIRs. The copy loop bursts (while space remains) until an end bit or
// a jump bit; on a jump bit it waits for the branch MIR's retirement
// (OnBranchResolved) and then resumes at the not-taken continuation or
// loads the shared taken block. The confirm to the decoder is held
// while a write_pc MIR has been copied but not yet landed
// (OnWritePcLanded), so the decoder resyncs to the jump target before
// the next instruction is decoded. Each FIFO entry carries pop_n -- the
// opcode bytes its retire pops (nonzero only for the first MIR of an
// instruction; the taken block carries none).
struct QueueEntry {
	MicroInstruction mir;
	Signals sig;
	uint8_t pop_n = 0;
};

class InstructionQueue {
public:
	void Init() {
		fifo_.clear();
		state_ = State::COPY;
		routine_done_ = true;  // force a decode on the first copy
		jump_pending_ = false;
		first_ = false;
		index_ = 0;
		pos_ = 0;
		pop_n_ = 0;
		resume_index_ = 0;
		resume_pos_ = 0;
	}

	// The front-end step: fill the FIFO (the burst copy) and drive the
	// confirm handshake with the decoder.
	void Cycle(Decoder& decoder, const InstructionFetchUnit& ifu,
	           const ControlMemory& cm) {
		if (state_ == State::WAIT_BRANCH) {
			return;
		}
		while (fifo_.size() < QUEUE_DEPTH) {
			if (routine_done_) {
				if (jump_pending_) {
					return;  // hold the confirm until the write_pc lands
				}
				uint32_t index = 0;
				uint8_t pop = 0;
				if (!decoder.Decode(ifu, index, pop)) {
					return;  // the bytes are not fetched yet
				}
				index_ = index;
				pos_ = 0;
				pop_n_ = pop;
				first_ = true;
				routine_done_ = false;
			}
			MicroInstruction mir = cm.FetchMIR(index_ + pos_);
			pos_ += 1;
			if (mir.GetSignals().write_pc) {
				jump_pending_ = true;
			}
			QueueEntry e;
			e.mir = mir;
			e.sig = mir.GetSignals();
			e.pop_n = first_ ? pop_n_ : 0;
			first_ = false;
			fifo_.push_back(e);
			if (mir.GetJumpBit()) {
				state_ = State::WAIT_BRANCH;
				resume_index_ = index_;
				resume_pos_ = pos_;
				return;
			}
			if (mir.GetEndBit()) {
				routine_done_ = true;
			}
		}
	}

	// The branch MIR retired: load the shared taken block or resume the
	// copy at the not-taken continuation.
	void OnBranchResolved(bool taken) {
		assert(state_ == State::WAIT_BRANCH);
		state_ = State::COPY;
		first_ = false;
		routine_done_ = false;
		if (taken) {
			index_ = TAKEN_BLOCK_INDEX;
			pos_ = 0;
		} else {
			index_ = resume_index_;
			pos_ = resume_pos_;
		}
	}

	// A copied write_pc MIR landed its jump: the confirm may proceed
	// (the decoder has resynced).
	void OnWritePcLanded() {
		jump_pending_ = false;
	}

	bool HasHead() const {
		return !fifo_.empty();
	}

	QueueEntry PeekHead() const {
		assert(!fifo_.empty());
		return fifo_.front();
	}

	void PopHead() {
		assert(!fifo_.empty());
		fifo_.pop_front();
	}

	size_t GetSize() const {
		return fifo_.size();
	}

private:
	enum class State {
		COPY,
		WAIT_BRANCH
	};

	std::deque<QueueEntry> fifo_;
	State state_ = State::COPY;
	bool routine_done_ = true;
	bool jump_pending_ = false;
	bool first_ = false;
	uint32_t index_ = 0;
	uint32_t pos_ = 0;
	uint8_t pop_n_ = 0;
	uint32_t resume_index_ = 0;
	uint32_t resume_pos_ = 0;
};
