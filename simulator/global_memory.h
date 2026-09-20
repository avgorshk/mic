#pragma once

#include <stdint.h>

#include <cassert>
#include <cstring>
#include <vector>

constexpr uint32_t GLOBAL_MEMORY_SIZE    = 1024 * 1024; // 1 MB
constexpr uint32_t PROGRAM_SEGMENT_SIZE  = 64 * 1024;   // 64 KB
constexpr uint32_t CONSTANT_SEGMENT_SIZE = 64 * 1024;   // 64 KB
constexpr uint32_t DATA_SEGMENT_SIZE =
	GLOBAL_MEMORY_SIZE - PROGRAM_SEGMENT_SIZE - CONSTANT_SEGMENT_SIZE;

class GlobalMemory {
public:
	GlobalMemory() : memory_(GLOBAL_MEMORY_SIZE) {}

public:
	void SetProgram(const std::vector<uint8_t>& program) {
		assert(program.size() <= PROGRAM_SEGMENT_SIZE);
		memcpy(
			memory_.data(), program.data(),
			program.size() * sizeof(uint8_t));
	}

	void SetData(const std::vector<uint32_t>& data) {
		assert(data.size() * sizeof(uint32_t) <= DATA_SEGMENT_SIZE);
		memcpy(
			memory_.data() + PROGRAM_SEGMENT_SIZE + CONSTANT_SEGMENT_SIZE,
			data.data(), data.size() * sizeof(uint32_t));
	}

	uint32_t GetPC() const {
		return 0;
	}

	uint32_t GetData() const {
		uint32_t data = PROGRAM_SEGMENT_SIZE + CONSTANT_SEGMENT_SIZE;
		assert((data & (sizeof(uint32_t) - 1)) == 0);
		return data / sizeof(uint32_t);
	}

	uint32_t GetCPP() const {
		uint32_t cpp = PROGRAM_SEGMENT_SIZE;
		assert((cpp & (sizeof(uint32_t) - 1)) == 0);
		return cpp / sizeof(uint32_t);
	}

	uint8_t Fetch(uint8_t is_fetch, uint32_t pc, uint8_t& is_written) {
		if (is_fetch_) {
			assert(fetch_pc_ < PROGRAM_SEGMENT_SIZE);
			uint8_t mbr = memory_[fetch_pc_];
			is_fetch_ = 0;
			if (is_fetch) {
				is_fetch_ = 1;
				assert(pc < PROGRAM_SEGMENT_SIZE);
				fetch_pc_ = pc;
			}
			is_written = 1;
			return mbr;
		}
		if (is_fetch) {
			is_fetch_ = 1;
			assert(pc < PROGRAM_SEGMENT_SIZE);
			fetch_pc_ = pc;
		}
		is_written = 0;
		return 0;
	}

	uint32_t Read(uint8_t is_read, uint32_t mar, uint8_t& is_written) {
		if (is_read_) {
			assert(read_addr_ * sizeof(uint32_t) < memory_.size());
			uint32_t* ptr = reinterpret_cast<uint32_t*>(memory_.data());
			uint32_t mdr = ptr[read_addr_];
			if (is_read) {
				is_read_ = 1;
				assert(mar * sizeof(uint32_t) < memory_.size());
				read_addr_ = mar;
			}
			is_written = 1;
			return mdr;
		}
		if (is_read) {
			is_read_ = 1;
			assert(mar * sizeof(uint32_t) < memory_.size());
			read_addr_ = mar;
		}
		is_written = 0;
		return 0;
	}

private:
	std::vector<uint8_t> memory_;

	uint8_t is_fetch_ = 0;
	uint32_t fetch_pc_ = 0;
	uint8_t is_read_ = 0;
	uint32_t read_addr_ = 0;
	uint8_t is_write_ = 0;
	uint32_t write_addr_ = 0;
};