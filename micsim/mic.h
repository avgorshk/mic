#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>

#include <windows.h>

#include "alu.h"
#include "decoder.h"
#include "global_memory.h"
#include "ifu.h"
#include "instruction_queue.h"
#include "signals.h"

// The PC and MBR now live inside the InstructionFetchUnit. The Registers
// struct keeps pc/mbr as READ-ONLY views (mirrored each cycle) so tests
// and callers can observe them.
struct Registers {
	uint32_t mar;
	uint32_t mdr;
	uint32_t pc;
	uint32_t mbr;
	uint32_t sp;
	uint32_t lv;
	uint32_t cpp;
	uint32_t tos;
	uint32_t opc;
	uint32_t h;
};

// One 4-slot datapath conveyor slot. Each slot carries its MIR, signals,
// the opcode-byte pop count of its retire, and the result registers it
// owns:
//   a/b   latched at MIR1, consumed by the ALU at MIR2
//   c/n/z ALU result: written back at the MIR's MIR3 stage-4, carried
//         through MIR4 solely as the gap-2 forward source (tap 2)
//   mar/mdr/rd_pending/wr_pending: memory ops recorded at MIR3,
//         performed at MIR4 (slot-resident: GlobalMemory is
//         synchronous)
struct PipelineSlot {
	bool valid = false;
	MicroInstruction mir;
	Signals sig{ 0 };
	uint8_t pop_n = 0;
	uint32_t a = 0;
	uint32_t b = 0;
	uint32_t c = 0;
	uint8_t n = 0;
	uint8_t z = 0;
	uint32_t mar = 0;
	uint32_t mdr = 0;
	bool rd_pending = false;
	bool wr_pending = false;
};

// The 7-stage machine: IFU -> Instruction Decoder -> Instruction Queue
// -> Operands Read (MIR1) -> ALU (MIR2) -> WriteBack (MIR3) -> MemRead
// (MIR4). The queue -> MIR1 handoff is the single gating point: a MIR
// is delivered only when its operand bytes are in the byte queue and
// (for MDR readers) no memory read is in flight. The retire pops the
// routine's opcode bytes, consumes the operand bytes, resolves split
// branches, and halts on the halt bit. Busy = retiring cycles, Empty =
// total - Busy, exactly.
class MIC {
public:
	MIC(GlobalMemory* global_memory) {
		global_memory_ = global_memory;

		regs_.cpp = global_memory_->GetConstantSegmentAddress();
		regs_.lv = global_memory_->GetDataSegmentAddress();
		regs_.sp = regs_.lv;

		ifu_.Init(global_memory_, global_memory_->GetProgramSegmentAddress());
		decoder_.Resync(ifu_.GetPC());
		queue_.Init();
		regs_.pc = ifu_.GetPC();
		regs_.mbr = ifu_.Read1();

		char trace_env[8] = { 0 };
		GetEnvironmentVariableA("MIC_TRACE", trace_env, sizeof(trace_env));
		if (trace_env[0] != 0) {
			trace_ = true;
		}
	}

public:
	// One machine cycle = one micro-clock: the front-end (decoder +
	// queue) fills the FIFO, the queue delivers its head into MIR1, and
	// the four datapath slots execute their stages in parallel,
	// M1 -> M2 -> M3 -> M4 in-cycle.
	bool Cycle() {
		// Stage 0: the front-end fills the FIFO.
		queue_.Cycle(decoder_, ifu_, control_memory_);

		// Stage 1: the queue -> MIR1 handoff (the single gate point).
		Fetch();

		// Stages 2-5: the datapath conveyor.
		LatchS1();
		ExecuteS2();
		RetireS3();
		Stage4();
		Advance();

		if (trace_) {
			std::printf(
				"cyc M1=%c M2=%c M3=%c M4=%c | fifo=%u q=%u walk=%u "
				"| sp=%u tos=%u lv=%u mar=%u mdr=%u opc=%u h=%u "
				"pc=%u mbr=%02X | stall=%llu retire=%llu halt=%d\n",
				s1_.valid ? 'v' : '-',
				s2_.valid ? 'v' : '-',
				s3_.valid ? 'v' : '-',
				s4_.valid ? 'v' : '-',
				(unsigned)queue_.GetSize(),
				ifu_.Available(),
				decoder_.GetWalk(),
				regs_.sp, regs_.tos, regs_.lv, regs_.mar,
				regs_.mdr, regs_.opc, regs_.h,
				regs_.pc, ifu_.Read1(),
				(unsigned long long)stall_cycles_,
				(unsigned long long)retire_cycles_,
				halt_ ? 1 : 0);
		}

		return halt_;
	}

	void SetSP(uint32_t sp) {
		regs_.sp = sp;
	}

	void SetTOS(uint32_t tos) {
		regs_.tos = tos;
	}

	void SetLV(uint32_t lv) {
		regs_.lv = lv;
	}

	Registers GetRegisters() const {
		return regs_;
	}

	// Delivery-gate events: a cycle where the queue head was held or
	// the FIFO was empty (debug aid, not part of the accounting).
	uint64_t GetStallCycles() const {
		return stall_cycles_;
	}

	// Cycles where the MIR3 slot retired a MIR (every MIR of the
	// program retires exactly once) = the busy cycles.
	uint64_t GetRetireCycles() const {
		return retire_cycles_;
	}

private:
	// ---------------------------------------------------------- helpers

	// Operand bytes a MIR consumes at its retire (MBR2 reads consume 2).
	static uint32_t Opsize(const Signals& sig) {
		switch (sig.a_read) {
		case AREAD_MBR_SIGNED:
		case AREAD_MBR_UNSIGNED:
			return 1;
		case AREAD_MBR2_SIGNED:
		case AREAD_MBR2_UNSIGNED:
			return 2;
		default:
			return 0;
		}
	}

	// Bytes a slot's MIR consumes at its retire: its operands.
	static uint32_t Need(const PipelineSlot& s) {
		if (!s.valid) return 0;
		return Opsize(s.sig);
	}

	// True when the ALU actually consumes MDR as an input. Gated on
	// ena/enb: a pad MIR reads the bus mux but the ALU ignores it.
	static bool ReadsMdr(const Signals& sig) {
		if (sig.a_read == AREAD_MDR && sig.alu.ena) return true;
		if (sig.b_read == BREAD_MDR && sig.alu.enb) return true;
		return false;
	}

	// True when the producer writes the register the consumer's A input
	// reads (the register forwarding match, taps 1/2).
	static bool ProducesA(const Signals& producer, const Signals& consumer) {
		if (!consumer.alu.ena) return false;
		switch (consumer.a_read) {
		case AREAD_MDR: return producer.write_mdr != 0;
		case AREAD_SP:  return producer.write_sp != 0;
		case AREAD_LV:  return producer.write_lv != 0;
		case AREAD_CPP: return producer.write_cpp != 0;
		case AREAD_TOS: return producer.write_tos != 0;
		case AREAD_OPC: return producer.write_opc != 0;
		case AREAD_H:   return producer.write_h != 0;
		default:        return false;  // MBR sources come from the IFU
		}
	}

	static bool ProducesB(const Signals& producer, const Signals& consumer) {
		if (!consumer.alu.enb) return false;
		switch (consumer.b_read) {
		case BREAD_MDR: return producer.write_mdr != 0;
		case BREAD_SP:  return producer.write_sp != 0;
		case BREAD_LV:  return producer.write_lv != 0;
		case BREAD_CPP: return producer.write_cpp != 0;
		case BREAD_TOS: return producer.write_tos != 0;
		case BREAD_OPC: return producer.write_opc != 0;
		case BREAD_H:   return producer.write_h != 0;
		default:        return false;  // BREAD_PC comes from the IFU
		}
	}

	// ------------------------------------------------------------- fetch

	// Stage 1: deliver the queue head into MIR1. The delivery holds
	// (MIR1 empty, everything else advances) when:
	// (a) the head MIR reads MDR while a memory read is in flight (the
	//     delivery lands at the END of the reader's MIR4 cycle -- a
	//     consumer delivered that cycle would latch before it), or
	// (b) the IFU queue cannot satisfy the latch's byte needs yet (the
	//     post-jump refill race and the decoder's lookahead are both
	//     handled by this one check).
	void Fetch() {
		s1_.valid = false;

		if (!queue_.HasHead()) {
			++stall_cycles_;
			return;
		}
		QueueEntry head = queue_.PeekHead();
		Signals sig = head.sig;

		bool mdr_gated = ReadsMdr(sig)
			&& ((s2_.valid && s2_.sig.mem_rd)
				|| (s3_.valid && s3_.sig.mem_rd)
				|| (s4_.valid && s4_.sig.mem_rd));

		// (c) the jump gate: a MIR with operand bytes must not latch
		// while a write_pc MIR is about to land (its MIR2/MIR3 slot):
		// the jump flushes the byte queue out from under the latch and
		// the retire's consumption. After the landing the byte-
		// availability check covers the refill race.
		bool jump_gated = Opsize(sig) > 0
			&& ((s2_.valid && s2_.sig.write_pc)
				|| (s3_.valid && s3_.sig.write_pc));

		// The operand bytes sit behind: the head's own un-popped opcode
		// bytes, the older occupants' un-popped opcode bytes (consumed
		// at their retires), and the older occupants' yet-unconsumed
		// operand bytes.
		peek_index_ = head.pop_n
			+ (s2_.valid ? s2_.pop_n : 0)
			+ (s3_.valid ? s3_.pop_n : 0)
			+ Need(s2_) + Need(s3_);
		uint32_t opsize = Opsize(sig);
		bool enough = ifu_.Available() >= peek_index_ + opsize;

		if (!mdr_gated && !jump_gated && enough) {
			s1_.valid = true;
			s1_.mir = head.mir;
			s1_.sig = sig;
			s1_.pop_n = head.pop_n;
			s1_.a = 0;
			s1_.b = 0;
			s1_.c = 0;
			s1_.n = 0;
			s1_.z = 0;
			s1_.mar = 0;
			s1_.mdr = 0;
			s1_.rd_pending = false;
			s1_.wr_pending = false;
			queue_.PopHead();
		} else {
			++stall_cycles_;
			if (trace_) {
				std::printf("  fetch held: mdr=%d jump=%d enough=%d\n",
					mdr_gated ? 1 : 0, jump_gated ? 1 : 0,
					enough ? 1 : 0);
			}
		}
	}

	// ------------------------------------------------------------- latch

	// Stage 2: mux the A/B buses and latch into the MIR1 occupant's a/b.
	// MBR reads peek at peek_index_ WITHOUT consuming -- the
	// consumption happens at the MIR's retire (MIR3).
	void LatchS1() {
		if (!s1_.valid) return;

		uint32_t bus_a = 0;
		switch (s1_.sig.a_read) {
		case AREAD_MDR:
			bus_a = regs_.mdr;
			break;
		case AREAD_MBR_SIGNED:
			bus_a = (uint32_t)ifu_.PeekAtSigned(peek_index_);
			break;
		case AREAD_MBR_UNSIGNED:
			bus_a = ifu_.PeekAt(peek_index_);
			break;
		case AREAD_MBR2_SIGNED:
			bus_a = (uint32_t)ifu_.Peek2AtSigned(peek_index_);
			break;
		case AREAD_MBR2_UNSIGNED:
			bus_a = ifu_.Peek2At(peek_index_);
			break;
		case AREAD_SP:
			bus_a = regs_.sp;
			break;
		case AREAD_LV:
			bus_a = regs_.lv;
			break;
		case AREAD_CPP:
			bus_a = regs_.cpp;
			break;
		case AREAD_TOS:
			bus_a = regs_.tos;
			break;
		case AREAD_OPC:
			bus_a = regs_.opc;
			break;
		case AREAD_H:
			bus_a = regs_.h;
			break;
		default:
			assert(0);
		}

		uint32_t bus_b = 0;
		switch (s1_.sig.b_read) {
		case BREAD_MDR:
			bus_b = regs_.mdr;
			break;
		case BREAD_SP:
			bus_b = regs_.sp;
			break;
		case BREAD_LV:
			bus_b = regs_.lv;
			break;
		case BREAD_CPP:
			bus_b = regs_.cpp;
			break;
		case BREAD_TOS:
			bus_b = regs_.tos;
			break;
		case BREAD_OPC:
			bus_b = regs_.opc;
			break;
		case BREAD_PC:
			// The IFU's PC plus the bytes the older occupants will
			// consume before this MIR's operands reach the queue
			// front: the PC as the microcode expects it.
			bus_b = ifu_.GetPC() + peek_index_;
			break;
		case BREAD_H:
			bus_b = regs_.h;
			break;
		default:
			assert(0);
		}

		s1_.a = bus_a;
		s1_.b = bus_b;
	}

	// --------------------------------------------------------------- ALU

	// Stage 3: the ALU runs on the latched A/B with the forward taps
	// applied. Tap 1: MIR3's c (gap-1 producer -- its writeback lands at
	// stage 4 of THIS cycle, after this ALU). Tap 2: MIR4's carried c
	// (gap-2 producer -- its writeback was after the consumer's latch).
	// The youngest producer wins (MIR3 before MIR4).
	void ExecuteS2() {
		if (!s2_.valid) return;

		uint32_t a = s2_.a;
		uint32_t b = s2_.b;

		if (s3_.valid && ProducesA(s3_.sig, s2_.sig)) {
			a = s3_.c;
		} else if (s4_.valid && ProducesA(s4_.sig, s2_.sig)) {
			a = s4_.c;
		}
		if (s3_.valid && ProducesB(s3_.sig, s2_.sig)) {
			b = s3_.c;
		} else if (s4_.valid && ProducesB(s4_.sig, s2_.sig)) {
			b = s4_.c;
		}

		alu_.SetFunction(s2_.sig.alu);
		alu_.SetAB(a, b);
		alu_.Execute();
		alu_.Shift();

		s2_.c = alu_.GetC();
		s2_.n = alu_.GetN();
		s2_.z = alu_.GetZ();
	}

	// ------------------------------------------------------------- retire

	// Stage 4: retire the MIR3 occupant -- pop the routine's opcode
	// bytes, record the memory ops, consume the operand bytes, resolve
	// split branches, halt. The delivery gates guarantee every byte is
	// available; the consumption order keeps the queue front aligned.
	void RetireS3() {
		if (!s3_.valid) return;

		// Opcode pop: the routine's first MIR pops its instruction's
		// opcode bytes (1, or 2 for wide), then its operands -- the
		// opcode precedes the operands in the byte stream.
		if (s3_.pop_n > 0) {
			assert(ifu_.Available() >= s3_.pop_n);
			ifu_.Consume(s3_.pop_n);
		}

		// Memory issue: the effective address/data come from the slot's
		// own C when this MIR writes the register in the same cycle.
		uint32_t mar_eff = s3_.sig.write_mar ? s3_.c : regs_.mar;
		uint32_t mdr_eff = s3_.sig.write_mdr ? s3_.c : regs_.mdr;
		s3_.mar = mar_eff;
		s3_.mdr = mdr_eff;
		s3_.rd_pending = s3_.sig.mem_rd != 0;
		s3_.wr_pending = s3_.sig.mem_wr != 0;

		// Operand consumption (the delivery check guaranteed the bytes).
		switch (s3_.sig.a_read) {
		case AREAD_MBR_SIGNED:
		case AREAD_MBR_UNSIGNED:
			ifu_.Consume(1);
			break;
		case AREAD_MBR2_SIGNED:
		case AREAD_MBR2_UNSIGNED:
			ifu_.Consume(2);
			break;
		default:
			break;
		}

		// Branch resolution: the dedicated branch MIR evaluates its JAM
		// flags against the predecessor's N/Z carried in the MIR4 slot
		// (the branch MIR is adjacent to the flag-computing MIR, so the
		// MIR4 occupant is always that predecessor).
		if (s3_.mir.GetJumpBit()) {
			bool taken = (s3_.mir.GetJAMN() && s4_.n)
				|| (s3_.mir.GetJAMZ() && s4_.z);
			queue_.OnBranchResolved(taken);
			if (trace_) {
				std::printf("  branch resolved: taken=%d\n",
					taken ? 1 : 0);
			}
		}

		if (s3_.mir.GetHalt()) {
			halt_ = true;
		}

		++retire_cycles_;
	}

	// -------------------------------------------------------------- stage4

	// Stage 5: memory + writeback + IFU + front-end resync. The order
	// is load-bearing:
	// (1) MIR4's pending ops deliver/commit, (2) MIR3's writeback
	// applies -- the younger MIR wins the MDR register (e.g. SWAP1's
	// read delivery vs SWAP4's write_mdr in the same cycle), (3) a
	// write_pc jump flushes the IFU and resyncs the decoder, (4) the
	// IFU refills.
	void Stage4() {
		if (s4_.valid) {
			if (s4_.wr_pending) {
				global_memory_->WriteNow(s4_.mar, s4_.mdr);
			}
			if (s4_.rd_pending) {
				regs_.mdr = global_memory_->ReadNow(s4_.mar);
			}
		}

		if (s3_.valid) {
			if (s3_.sig.write_cpp) regs_.cpp = s3_.c;
			if (s3_.sig.write_h)   regs_.h = s3_.c;
			if (s3_.sig.write_lv)  regs_.lv = s3_.c;
			if (s3_.sig.write_mar) regs_.mar = s3_.c;
			if (s3_.sig.write_mdr) regs_.mdr = s3_.c;
			if (s3_.sig.write_opc) regs_.opc = s3_.c;
			if (s3_.sig.write_sp)  regs_.sp = s3_.c;
			if (s3_.sig.write_tos) regs_.tos = s3_.c;
			if (s3_.sig.write_pc) {
				ifu_.Jump(s3_.c);
				queue_.OnWritePcLanded();
				decoder_.Resync(ifu_.GetPC());
			}
		}

		ifu_.Cycle();

		regs_.pc = ifu_.GetPC();
		regs_.mbr = ifu_.Read1();
	}

	// ---------------------------------------------------------- advancement

	// Copy the full slot state (result registers included); s1_ is
	// overwritten by the next Fetch().
	void Advance() {
		s4_ = s3_;
		s3_ = s2_;
		s2_ = s1_;
	}

private:
	Registers regs_ = { 0 };
	ALU alu_;
	ControlMemory control_memory_;
	Decoder decoder_;
	InstructionQueue queue_;
	InstructionFetchUnit ifu_;
	GlobalMemory* global_memory_ = nullptr;

	PipelineSlot s1_;
	PipelineSlot s2_;
	PipelineSlot s3_;
	PipelineSlot s4_;

	uint32_t peek_index_ = 0;
	bool halt_ = false;
	bool trace_ = false;
	uint64_t stall_cycles_ = 0;
	uint64_t retire_cycles_ = 0;
};
