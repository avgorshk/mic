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
		uint32_t bus_b_ = 0;
		if (signals_.read_cpp) bus_b_ = regs_.cpp;
		if (signals_.read_lv) bus_b_ = regs_.lv;
		if (signals_.read_mdr) bus_b_ = regs_.mdr;
		if (signals_.read_opc) bus_b_ = regs_.opc;
		if (signals_.read_pc) bus_b_ = regs_.pc;
		if (signals_.read_sp) bus_b_ = regs_.sp;
		if (signals_.read_tos) bus_b_ = regs_.tos;
		if (signals_.read_mbr_unsigned) bus_b_ = regs_.mbr;
		if (signals_.read_mbr_signed) {
			uint32_t sign = (regs_.mbr >> 7);
			bus_b_ = 0;
			if (sign == 1) {
				bus_b_ = 0xFFFFFF00;
			}
			bus_b_ |= regs_.mbr;
		}
		alu_.SetInput(regs_.h, bus_b_);
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

		control_memory_.UpdateMPC(alu_.GetN(), alu_.GetZ(), regs_.mbr);
	}

private:
	Registers regs_ = { 0 };
	Signals signals_ = { 0 };
	ALU alu_;
	ControlMemory control_memory_;
	GlobalMemory* global_memory_ = nullptr;
	bool halt_ = false;
};