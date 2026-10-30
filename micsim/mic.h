#pragma once

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
	}

public:
	bool Cycle() {
		SetSignals();

		// The IFU must satisfy this cycle's byte needs (operand reads
		// plus one byte for a JMPC dispatch). If not, the machine holds:
		// no register writes, no MPC update -- the MIR retries next
		// cycle while the refill state machine keeps running.
		uint32_t need = ComputeFetchNeed();
		if (ifu_.Stall(need)) {
			++stall_cycles_;
			ifu_.Cycle();
			return halt_;
		}

		ReadRegisters();
		RunALU();
		WriteRegisters();
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

private:
	uint32_t ComputeFetchNeed() const {
		uint32_t need = 0;
		switch (signals_.a_read) {
		case AREAD_MBR_SIGNED:
		case AREAD_MBR_UNSIGNED:
			need = 1;
			break;
		case AREAD_MBR2_SIGNED:
		case AREAD_MBR2_UNSIGNED:
			need = 2;
			break;
		default:
			break;
		}
		// The dispatch consumes one byte (the next opcode) at the edge.
		if (jmpc_) {
			need += 1;
		}
		return need;
	}

	void SetSignals() {
		MicroInstruction inst = control_memory_.LoadMIR();
		signals_ = inst.GetSignals();
		alu_.SetFunction(signals_.alu);
		halt_ = inst.GetHalt();
		jmpc_ = inst.GetJMPC();
	}

	void ReadRegisters() {
		uint32_t bus_a_ = 0;
		switch (signals_.a_read) {
		case AREAD_MDR:
			bus_a_ = regs_.mdr;
			break;
		case AREAD_MBR_SIGNED:
			bus_a_ = ifu_.Read1Signed();
			break;
		case AREAD_MBR_UNSIGNED:
			bus_a_ = ifu_.Read1();
			break;
		case AREAD_MBR2_SIGNED:
			bus_a_ = ifu_.Read2Signed();
			break;
		case AREAD_MBR2_UNSIGNED:
			bus_a_ = ifu_.Read2();
			break;
		case AREAD_SP:
			bus_a_ = regs_.sp;
			break;
		case AREAD_LV:
			bus_a_ = regs_.lv;
			break;
		case AREAD_CPP:
			bus_a_ = regs_.cpp;
			break;
		case AREAD_TOS:
			bus_a_ = regs_.tos;
			break;
		case AREAD_OPC:
			bus_a_ = regs_.opc;
			break;
		case AREAD_H:
			bus_a_ = regs_.h;
			break;
		default:
			assert(0);
		}

		uint32_t bus_b_ = 0;
		switch (signals_.b_read) {
		case BREAD_MDR:
			bus_b_ = regs_.mdr;
			break;
		case BREAD_SP:
			bus_b_ = regs_.sp;
			break;
		case BREAD_LV:
			bus_b_ = regs_.lv;
			break;
		case BREAD_CPP:
			bus_b_ = regs_.cpp;
			break;
		case BREAD_TOS:
			bus_b_ = regs_.tos;
			break;
		case BREAD_OPC:
			bus_b_ = regs_.opc;
			break;
		case BREAD_PC:
			// The IFU's PC: the address of the byte currently in MBR1.
			bus_b_ = ifu_.GetPC();
			break;
		case BREAD_H:
			bus_b_ = regs_.h;
			break;
		default:
			assert(0);
		}

		alu_.SetInput(bus_a_, bus_b_);
	}

	void RunALU() {
		alu_.Execute();
		alu_.Shift();
	}

	void WriteRegisters() {
		uint32_t bus_c_ = alu_.GetResult();
		if (signals_.write_cpp) regs_.cpp = bus_c_;
		if (signals_.write_h) regs_.h = bus_c_;
		if (signals_.write_lv) regs_.lv = bus_c_;
		if (signals_.write_mar) regs_.mar = bus_c_;
		if (signals_.write_mdr) regs_.mdr = bus_c_;
		if (signals_.write_opc) regs_.opc = bus_c_;
		if (signals_.write_sp) regs_.sp = bus_c_;
		if (signals_.write_tos) regs_.tos = bus_c_;

		global_memory_->Write(signals_.mem_wr, regs_.mar, regs_.mdr);
		uint8_t is_written = 0;
		uint32_t mdr = global_memory_->Read(signals_.mem_rd, regs_.mar, is_written);
		if (is_written) {
			regs_.mdr = mdr;
		}

		// Operand consumption: an A-bus MBR read advances the queue.
		// Then the dispatch snapshot sees the byte that FOLLOWS the
		// consumed operand -- the next opcode -- and consumes it too.
		switch (signals_.a_read) {
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
		uint32_t mbr_for_dispatch = ifu_.Read1();
		if (jmpc_) {
			ifu_.Consume(1);
		}
		control_memory_.UpdateMPC(alu_.GetN(), alu_.GetZ(), mbr_for_dispatch);

		// Program-counter change (branch / invokevirtual / ireturn):
		// flush the queue and refill from the target.
		if (signals_.write_pc) {
			ifu_.Jump(bus_c_);
		}

		ifu_.Cycle();

		regs_.pc = ifu_.GetPC();
		regs_.mbr = ifu_.Read1();
	}

private:
	Registers regs_ = { 0 };
	Signals signals_ = { 0 };
	ALU alu_;
	ControlMemory control_memory_;
	InstructionFetchUnit ifu_;
	GlobalMemory* global_memory_ = nullptr;
	bool halt_ = false;
	bool jmpc_ = false;
	uint64_t stall_cycles_ = 0;
};