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
	MIC() {}

public:
	void InitCycle() {
		regs_.pc = global_memory_.GetPC();
		regs_.cpp = global_memory_.GetCPP();
		regs_.lv = global_memory_.GetData();
		signals_.mem_fetch = 1;
		ReadRegisters();
		RunALU();
		WriteRegisters();
	}

	void Cycle() {
		SetSignals();
		ReadRegisters();
		RunALU();
		WriteRegisters();
	}

	void SetSP(uint32_t sp) {
		regs_.sp = global_memory_.GetData() + sp;
	}

	void SetTOS(uint32_t tos) {
		regs_.tos = tos;
	}

	Registers GetRegisters() const {
		return regs_;
	}

	void SetProgram(const std::vector<uint8_t>& program) {
		global_memory_.SetProgram(program);
	}

	void SetData(const std::vector<uint32_t>& data) {
		global_memory_.SetData(data);
	}

	std::vector<uint32_t> GetData(size_t size) {
		return global_memory_.GetData(size);
	}

private:
	void SetSignals() {
		MicroInstruction inst = control_memory_.LoadMIR();
		signals_ = inst.GetSignals();
		alu_.SetFunction(signals_.alu);
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
		uint8_t mbr = global_memory_.Fetch(signals_.mem_fetch, regs_.pc, is_written);
		if (is_written) {
			regs_.mbr = mbr;
		}
		uint32_t mdr = global_memory_.Read(signals_.mem_rd, regs_.mar, is_written);
		if (is_written) {
			regs_.mdr = mdr;
		}
		global_memory_.Write(signals_.mem_wr, regs_.mar, regs_.mdr);

		control_memory_.UpdateMPC(alu_.GetN(), alu_.GetZ(), regs_.mbr);
	}

private:
	Registers regs_ = { 0 };
	Signals signals_ = { 0 };
	ALU alu_;
	ControlMemory control_memory_;
	GlobalMemory global_memory_;
};