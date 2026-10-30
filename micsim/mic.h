#pragma once

#include "alu.h"
#include "global_memory.h"
#include "control_memory.h"
#include "signals.h"

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

		regs_.pc = global_memory_->GetProgramSegmentAddress();
		regs_.cpp = global_memory_->GetConstantSegmentAddress();
		regs_.lv = global_memory_->GetDataSegmentAddress();
		regs_.sp = regs_.lv;

		uint8_t is_written = 0;
		global_memory_->Fetch(1, regs_.pc, is_written);
		uint8_t mbr0 = global_memory_->Fetch(0, regs_.pc, is_written);
		if (is_written) {
			regs_.mbr = mbr0;
		}
	}

public:
	bool Cycle() {
		SetSignals();
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

private:
	void SetSignals() {
		MicroInstruction inst = control_memory_.LoadMIR();
		signals_ = inst.GetSignals();
		alu_.SetFunction(signals_.alu);
		halt_ = inst.GetHalt();
	}

	void ReadRegisters() {
		uint32_t bus_a_ = 0;
		switch (signals_.a_read) {
		case AREAD_MDR:
			bus_a_ = regs_.mdr;
			break;
		case AREAD_MBR_SIGNED: {
			uint32_t sign = (regs_.mbr >> 7);
			bus_a_ = (sign == 1) ? 0xFFFFFF00 : 0;
			bus_a_ |= regs_.mbr;
			break;
		}
		case AREAD_MBR_UNSIGNED:
			bus_a_ = regs_.mbr;
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
			bus_b_ = regs_.pc;
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
		// MPC and MBR latch at the same clock edge, so the dispatch must
		// see the pre-edge MBR: snapshot it BEFORE the memory deliveries.
		// A byte delivered this cycle can steer the dispatch no earlier
		// than the next cycle.
		uint32_t mbr_for_dispatch = regs_.mbr;

		uint32_t bus_c_ = alu_.GetResult();
		if (signals_.write_cpp) regs_.cpp = bus_c_;
		if (signals_.write_h) regs_.h = bus_c_;
		if (signals_.write_lv) regs_.lv = bus_c_;
		if (signals_.write_mar) regs_.mar = bus_c_;
		if (signals_.write_mdr) regs_.mdr = bus_c_;
		if (signals_.write_opc) regs_.opc = bus_c_;
		if (signals_.write_pc) regs_.pc = bus_c_;
		if (signals_.write_sp) regs_.sp = bus_c_;
		if (signals_.write_tos) regs_.tos = bus_c_;

		uint8_t is_written = 0;
		uint8_t mbr = global_memory_->Fetch(signals_.mem_fetch, regs_.pc, is_written);
		if (is_written) {
			regs_.mbr = mbr;
		}
		global_memory_->Write(signals_.mem_wr, regs_.mar, regs_.mdr);
		uint32_t mdr = global_memory_->Read(signals_.mem_rd, regs_.mar, is_written);
		if (is_written) {
			regs_.mdr = mdr;
		}

		control_memory_.UpdateMPC(alu_.GetN(), alu_.GetZ(), mbr_for_dispatch);
	}

private:
	Registers regs_ = { 0 };
	Signals signals_ = { 0 };
	ALU alu_;
	ControlMemory control_memory_;
	GlobalMemory* global_memory_ = nullptr;
	bool halt_ = false;
};