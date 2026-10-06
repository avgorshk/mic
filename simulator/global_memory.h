#pragma once

#include <stdint.h>

#include <cassert>
#include <cstring>
#include <vector>

class GlobalMemory {
public:
	GlobalMemory(
		uint32_t program_segment_size,
		uint32_t constant_segment_size,
		uint32_t data_segment_size)
		: program_segment_size_(program_segment_size),
		  constant_segment_size_(constant_segment_size),
		  data_segment_size_(data_segment_size) {
		memory_.resize(GetSize());
	}

public:
	uint32_t GetSize() const {
		return program_segment_size_ + constant_segment_size_ + data_segment_size_;
	}

	void SetProgram(const std::vector<uint8_t>& program) {
		assert(program.size() <= program_segment_size_);
		memcpy(
			memory_.data(), program.data(),
			program.size() * sizeof(uint8_t));
	}

	void SetData(const std::vector<uint32_t>& data) {
		assert(data.size() * sizeof(uint32_t) <= data_segment_size_);
		memcpy(
			memory_.data() + program_segment_size_ + constant_segment_size_,
			data.data(), data.size() * sizeof(uint32_t));
	}

	std::vector<uint32_t> GetData(size_t size) {
		assert(size * sizeof(uint32_t) < data_segment_size_);
		std::vector<uint32_t> data(size);
		memcpy(
			data.data(),
			memory_.data() + program_segment_size_ + constant_segment_size_,
			size * sizeof(uint32_t));
		return data;
	}

	uint32_t GetProgramSegmentAddress() const {
		return 0;
	}

	uint32_t GetConstantSegmentAddress() const {
		uint32_t cpp = program_segment_size_;
		assert((cpp & (sizeof(uint32_t) - 1)) == 0);
		return cpp / sizeof(uint32_t);
	}

	uint32_t GetDataSegmentAddress() const {
		uint32_t data = program_segment_size_ + constant_segment_size_;
		assert((data & (sizeof(uint32_t) - 1)) == 0);
		return data / sizeof(uint32_t);
	}

	uint8_t Fetch(uint8_t is_fetch, uint32_t pc, uint8_t& is_written) {
		uint8_t mbr = 0;
		is_written = 0;
		if (is_fetch_) {
			assert(fetch_pc_ < program_segment_size_);
			mbr = memory_[fetch_pc_];
			is_written = 1;
			is_fetch_ = 0;
		}
		if (is_fetch) {
			is_fetch_ = 1;
			assert(pc < program_segment_size_);
			fetch_pc_ = pc;
		}
		return mbr;
	}

	uint32_t Read(uint8_t is_read, uint32_t mar, uint8_t& is_written) {
		uint32_t mdr = 0;
		is_written = 0;
		if (is_read_) {
			assert(read_addr_ * sizeof(uint32_t) < memory_.size());
			uint32_t* ptr = reinterpret_cast<uint32_t*>(memory_.data());
			mdr = ptr[read_addr_];
			is_written = 1;
			is_read_ = 0;
		}
		if (is_read) {
			is_read_ = 1;
			assert(mar * sizeof(uint32_t) < memory_.size());
			read_addr_ = mar;
		}
		return mdr;
	}

	void Write(uint8_t is_write, uint32_t mar, uint32_t mdr) {
		if (is_write_) {
			assert(write_addr_ * sizeof(uint32_t) < memory_.size());
			uint32_t* ptr = reinterpret_cast<uint32_t*>(memory_.data());
			ptr[write_addr_] = write_data_;
			is_write_ = 0;
		}
		if (is_write) {
			is_write_ = 1;
			assert(mar * sizeof(uint32_t) < memory_.size());
			write_addr_ = mar;
			write_data_ = mdr;
		}
	}

private:
	uint32_t program_segment_size_ = 0;
	uint32_t constant_segment_size_ = 0;
	uint32_t data_segment_size_ = 0;
	std::vector<uint8_t> memory_;

	uint8_t is_fetch_ = 0;
	uint32_t fetch_pc_ = 0;
	uint8_t is_read_ = 0;
	uint32_t read_addr_ = 0;
	uint8_t is_write_ = 0;
	uint32_t write_addr_ = 0;
	uint32_t write_data_ = 0;
};
