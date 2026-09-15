#pragma once

#include <stdint.h>

#include <cassert>
#include <cstring>
#include <vector>

constexpr uint32_t GLOBAL_MEMORY_SIZE    = 1024 * 1024; // 1 MB
constexpr uint32_t PROGRAM_SEGMENT_SIZE  = 64 * 1024;   // 64 KB
constexpr uint32_t CONSTANT_SEGMENT_SIZE = 64 * 1024;   // 64 KB
constexpr uint32_t DATA_SIZE = GLOBAL_MEMORY_SIZE - PROGRAM_SEGMENT_SIZE - CONSTANT_SEGMENT_SIZE;

class GlobalMemory {
public:
	GlobalMemory()
		: memory_(GLOBAL_MEMORY_SIZE),
		  program_segment_(memory_.data()),
		  constant_segment_(reinterpret_cast<uint32_t*>(program_segment_ + PROGRAM_SEGMENT_SIZE)),
		  data_segment_(reinterpret_cast<uint32_t*>(program_segment_ + PROGRAM_SEGMENT_SIZE + CONSTANT_SEGMENT_SIZE)) {}

public:
	void SetProgram(const std::vector<uint8_t>& program) {
		assert(program.size() <= PROGRAM_SEGMENT_SIZE);
		memcpy(program_segment_, program.data(), program.size());
	}

	void SetData(const std::vector<uint32_t>& data) {
		assert(data.size() * sizeof(uint32_t) <= DATA_SIZE);
		memcpy(data_segment_, data.data(), data.size() * sizeof(uint32_t));
	}

	uint32_t GetPC() const {
		return 0;
	}

	uint32_t GetLV() const {
		return 0;
	}

	uint32_t GetCPP() const {
		return 0;
	}

	uint16_t Fetch(uint8_t is_fetch, uint32_t pc) {
		if (is_fetch_) {
			uint8_t mbr = program_segment_[fetch_pc_];
			is_fetch_ = 0;
			if (is_fetch) {
				is_fetch_ = 1;
				fetch_pc_ = pc;
			}
			return static_cast<uint16_t>(mbr);
		}
		if (is_fetch) {
			is_fetch_ = 1;
			fetch_pc_ = pc;
		}
		return 0xFFFF;
	}

private:
	std::vector<uint8_t> memory_;
	uint32_t* constant_segment_;
	uint32_t* data_segment_;
	uint8_t* program_segment_;

	uint8_t is_fetch_ = 0;
	uint32_t fetch_pc_ = 0;
	uint8_t is_read_ = 0;
	uint32_t read_addr_ = 0;
	uint8_t is_write_ = 0;
	uint32_t write_addr_ = 0;
};