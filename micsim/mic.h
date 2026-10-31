#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>

#include <windows.h>

#include "alu.h"
#include "control_memory.h"
#include "global_memory.h"
#include "ifu.h"
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

// One 4-stage conveyor slot. Each slot carries its MIR, signals, the
// microaddress it was fetched from, and the result registers it owns:
//   a/b   latched at S1, consumed by the ALU at S2
//   c/n/z ALU result: written back at the MIR's S3 stage-4, carried
//         through S4 solely as the gap-2 forward source (tap 2)
//   mar/mdr/rd_pending/wr_pending: memory ops recorded at S3, performed
//         at S4 (slot-resident: GlobalMemory is synchronous)
struct PipelineSlot {
	bool valid = false;
	uint32_t addr = 0;
	MicroInstruction mir;
	Signals sig{ 0 };
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

class MIC {
public:
	MIC(GlobalMemory* global_memory) {
		global_memory_ = global_memory;

		regs_.cpp = global_memory_->GetConstantSegmentAddress();
		regs_.lv = global_memory_->GetDataSegmentAddress();
		regs_.sp = regs_.lv;

		ifu_.Init(global_memory_, global_memory_->GetProgramSegmentAddress());
		regs_.pc = ifu_.GetPC();
		regs_.mbr = ifu_.Read1();

		// The decode step: NOP at 0x00 dispatches the first opcode.
		fetch_target_ = 0;

		char trace_env[8] = { 0 };
		GetEnvironmentVariableA("MIC_TRACE", trace_env, sizeof(trace_env));
		if (trace_env[0] != 0) {
			trace_ = true;
		}
	}

public:
	// One machine cycle = one micro-clock: the four slots execute their
	// stage in parallel, M1 -> M2 -> M3 -> M4 in-cycle.
	bool Cycle() {
		bool l = false;  // A/B assign (the S1 latch ran)
		bool a = false;  // the ALU ran
		bool c = false;  // C assign (the S3 writeback at stage 4)
		bool m = false;  // MDR read delivered / write committed (stage 4)

		if (!hold_) {
			Fetch();
		} else {
			hold_ = false;
		}
		l = s1_.valid;
		LatchS1();
		a = s2_.valid;
		ExecuteS2();
		if (!RetireS3()) {
			// Wholesale hold: the retire's bytes are not in the queue
			// (a jump flushed them out from under the in-flight MIRs).
			// The refill keeps running; nothing else advances; the
			// stages re-run next cycle (idempotent -- no consumes, no
			// writes happened).
			hold_ = true;
			ifu_.Cycle();
			++stall_cycles_;
			CountParts(l, a, false, false);
			if (trace_) {
				std::printf(
					"  retire hold: need=%u q=%u\n",
					Opsize(s3_.sig) + (s3_.mir.GetJMPC() ? 1 : 0),
					ifu_.Available());
			}
			return halt_;
		}
		c = s3_.valid;
		m = s4_.valid && (s4_.rd_pending || s4_.wr_pending);
		Stage4();
		Advance();
		CountParts(l, a, c, m);

		if (trace_) {
			std::printf(
				"cyc S1=%c%03X S2=%c%03X S3=%c%03X S4=%c%03X | "
				"q=%u idx=%u ft=%03X | sp=%u tos=%u lv=%u mar=%u "
				"mdr=%u opc=%u h=%u pc=%u mbr=%02X | stall=%llu halt=%d\n",
				s1_.valid ? 'v' : '-', s1_.addr,
				s2_.valid ? 'v' : '-', s2_.addr,
				s3_.valid ? 'v' : '-', s3_.addr,
				s4_.valid ? 'v' : '-', s4_.addr,
				ifu_.Available(), fetch_peek_index_, fetch_target_,
				regs_.sp, regs_.tos, regs_.lv, regs_.mar,
				regs_.mdr, regs_.opc, regs_.h,
				regs_.pc, ifu_.Read1(),
				(unsigned long long)stall_cycles_, halt_ ? 1 : 0);
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

	uint64_t GetStallCycles() const {
		return stall_cycles_;
	}

	// Cycles where the S3 slot retired a MIR (every MIR of the program
	// retires exactly once).
	uint64_t GetRetireCycles() const {
		return retire_cycles_;
	}

	// Per-part activity: a cycle counts a part when that part performed
	// its operation (the S1 latch assigned A/B, the ALU ran, the S3
	// writeback assigned C to the register file, the S4 slot delivered
	// an MDR read or committed a write). Busy = at least one part ran;
	// Busy + Empty == total cycles, exactly.
	uint64_t GetLatchCycles() const {
		return latch_cycles_;
	}

	uint64_t GetAluCycles() const {
		return alu_cycles_;
	}

	uint64_t GetCAssignCycles() const {
		return cassign_cycles_;
	}

	uint64_t GetMemCycles() const {
		return mem_cycles_;
	}

	uint64_t GetBusyCycles() const {
		return busy_cycles_;
	}

	uint64_t GetEmptyCycles() const {
		return empty_cycles_;
	}

private:
	// ---------------------------------------------------------- helpers

	// Per-cycle part accounting: Busy + Empty == total cycles, exactly.
	void CountParts(bool l, bool a, bool c, bool m) {
		if (l) ++latch_cycles_;
		if (a) ++alu_cycles_;
		if (c) ++cassign_cycles_;
		if (m) ++mem_cycles_;
		if (l || a || c || m) {
			++busy_cycles_;
		} else {
			++empty_cycles_;
		}
	}

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

	// Bytes a slot's MIR consumes at its retire: operands + the jmpc
	// snapshot byte.
	static uint32_t Need(const PipelineSlot& s) {
		if (!s.valid) return 0;
		return Opsize(s.sig) + (s.mir.GetJMPC() ? 1 : 0);
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

	// Stage 0: fetch into S1 along the static next_address chain. The
	// fetch holds (S1 empty, fetch target unchanged, everything else
	// advances) when:
	// (a) the fetched MIR reads MDR while a memory read is in flight
	//     (the delivery lands at the END of the reader's S4 cycle -- a
	//     consumer fetched that cycle would latch before it),
	// (b) the fetched MIR needs operand bytes while a write_pc MIR is
	//     in flight (the jump flushes the queue out from under the
	//     latch), or
	// (c) the IFU queue cannot satisfy the latch's byte needs yet.
	void Fetch() {
		s1_.valid = false;

		MicroInstruction mir = control_memory_.FetchMIR(fetch_target_);
		Signals sig = mir.GetSignals();

		bool mdr_gated = ReadsMdr(sig)
			&& ((s2_.valid && s2_.sig.mem_rd)
				|| (s3_.valid && s3_.sig.mem_rd)
				|| (s4_.valid && s4_.sig.mem_rd));
		bool jump_gated = Opsize(sig) > 0
			&& ((s2_.valid && s2_.sig.write_pc)
				|| (s3_.valid && s3_.sig.write_pc)
				|| (s4_.valid && s4_.sig.write_pc));

		// The operand bytes sit behind the older occupants' bytes: those
		// consumed at this cycle's retire plus next cycle's.
		fetch_peek_index_ = Need(s3_) + Need(s2_);
		uint32_t opsize = Opsize(sig);
		bool enough = ifu_.Available() >= fetch_peek_index_ + opsize;

		if (!mdr_gated && !jump_gated && enough) {
			s1_.valid = true;
			s1_.addr = fetch_target_;
			s1_.mir = mir;
			s1_.sig = sig;
			s1_.a = 0;
			s1_.b = 0;
			s1_.c = 0;
			s1_.n = 0;
			s1_.z = 0;
			s1_.mar = 0;
			s1_.mdr = 0;
			s1_.rd_pending = false;
			s1_.wr_pending = false;
			fetch_target_ = mir.GetNextAddress();
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

	// Stage 1: mux the A/B buses and latch into the S1 occupant's a/b.
	// MBR reads peek at fetch_peek_index_ WITHOUT consuming -- the
	// consumption happens at the MIR's retire (stage 3).
	void LatchS1() {
		if (!s1_.valid) return;

		uint32_t bus_a = 0;
		switch (s1_.sig.a_read) {
		case AREAD_MDR:
			bus_a = regs_.mdr;
			break;
		case AREAD_MBR_SIGNED:
			bus_a = (uint32_t)ifu_.PeekAtSigned(fetch_peek_index_);
			break;
		case AREAD_MBR_UNSIGNED:
			bus_a = ifu_.PeekAt(fetch_peek_index_);
			break;
		case AREAD_MBR2_SIGNED:
			bus_a = (uint32_t)ifu_.Peek2AtSigned(fetch_peek_index_);
			break;
		case AREAD_MBR2_UNSIGNED:
			bus_a = ifu_.Peek2At(fetch_peek_index_);
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
			bus_b = ifu_.GetPC() + fetch_peek_index_;
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

	// Stage 2: the ALU runs on the latched A/B with the forward taps
	// applied. Tap 1: S3's c (gap-1 producer -- its writeback lands at
	// stage 4 of THIS cycle, after this ALU). Tap 2: S4's carried c
	// (gap-2 producer -- its writeback was after the consumer's latch).
	// The youngest producer wins (S3 before S4).
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

	// Stage 3: retire the S3 occupant -- record the memory ops, consume
	// the operand bytes, resolve the dispatch, validate the fetch chain.
	// Returns false (wholesale hold) when the queue cannot satisfy the
	// retire's byte needs -- the jmpc byte of a post-jump dispatch may
	// still be in flight even though the fetch-time checks passed.
	bool RetireS3() {
		if (!s3_.valid) return true;

		uint32_t need = Opsize(s3_.sig) + (s3_.mir.GetJMPC() ? 1 : 0);
		if (ifu_.Available() < need) {
			return false;
		}

		// Memory issue: the effective address/data come from the slot's
		// own C when this MIR writes the register in the same cycle.
		uint32_t mar_eff = s3_.sig.write_mar ? s3_.c : regs_.mar;
		uint32_t mdr_eff = s3_.sig.write_mdr ? s3_.c : regs_.mdr;
		s3_.mar = mar_eff;
		s3_.mdr = mdr_eff;
		s3_.rd_pending = s3_.sig.mem_rd != 0;
		s3_.wr_pending = s3_.sig.mem_wr != 0;

		// Operand consumption (the fetch check guaranteed the bytes).
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

		// Dispatch: snapshot the byte AFTER the consumed operands, then
		// resolve the MPC (jmpc consumes the snapshot byte itself).
		uint32_t mbr_for_dispatch = ifu_.Read1();
		if (s3_.mir.GetJMPC()) {
			ifu_.Consume(1);
		}
		uint32_t resolved = s3_.mir.GetNextAddress();
		if (s3_.mir.GetJAMN()) resolved |= (s3_.n << 8);
		if (s3_.mir.GetJAMZ()) resolved |= (s3_.z << 8);
		if (s3_.mir.GetJMPC()) resolved |= (mbr_for_dispatch & 0xFF);

		// Chain validation: the fetched successor (S2's occupant) must
		// match the resolved one; otherwise flush the two speculative
		// slots and redirect the fetch stream.
		if (!s2_.valid || s2_.addr != resolved) {
			s1_.valid = false;
			s2_.valid = false;
			fetch_target_ = resolved;
			stall_cycles_ += 2;
			if (trace_) {
				std::printf("  flush: resolved=%03X s2=%03X\n",
					resolved, s2_.addr);
			}
		}

		if (s3_.mir.GetHalt()) {
			halt_ = true;
		}

		++retire_cycles_;
		return true;
	}

	// -------------------------------------------------------------- stage4

	// Stage 4: memory + writeback + IFU. The order is load-bearing:
	// (1) S4's pending ops deliver/commit, (2) S3's writeback applies --
	// the younger MIR wins the MDR register (e.g. SWAP1's read delivery
	// vs SWAP4's write_mdr in the same cycle), (3) the IFU refills.
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
			if (s3_.sig.write_pc)  ifu_.Jump(s3_.c);
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
	InstructionFetchUnit ifu_;
	GlobalMemory* global_memory_ = nullptr;

	PipelineSlot s1_;
	PipelineSlot s2_;
	PipelineSlot s3_;
	PipelineSlot s4_;

	uint32_t fetch_target_ = 0;
	uint32_t fetch_peek_index_ = 0;
	bool halt_ = false;
	bool hold_ = false;
	bool trace_ = false;
	uint64_t stall_cycles_ = 0;
	uint64_t retire_cycles_ = 0;
	uint64_t latch_cycles_ = 0;
	uint64_t alu_cycles_ = 0;
	uint64_t cassign_cycles_ = 0;
	uint64_t mem_cycles_ = 0;
	uint64_t busy_cycles_ = 0;
	uint64_t empty_cycles_ = 0;
};
