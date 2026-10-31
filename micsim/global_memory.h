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
		// +8 bytes of slack so the IFU's last 4-byte block read (which
		// may start at any byte address and extend past the program)
		// never leaves the physical storage.
		memory_.resize(GetSize() + 8);
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

	void SetConstants(const std::vector<uint32_t>& constants) {
		assert(constants.size() * sizeof(uint32_t) <= constant_segment_size_);
		memcpy(
			memory_.data() + program_segment_size_,
			constants.data(), constants.size() * sizeof(uint32_t));
	}

	void SetData(const std::vector<uint32_t>& data) {
		assert(data.size() * sizeof(uint32_t) <= data_segment_size_);
		memcpy(
			memory_.data() + program_segment_size_ + constant_segment_size_,
			data.data(), data.size() * sizeof(uint32_t));
	}

	std::vector<uint32_t> GetData(size_t size) {
		assert(size * sizeof(uint32_t) <= data_segment_size_);
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

	// Synchronous word access for the pipelined machine: the memory
	// latency lives in the pipeline slots (issue at S3, deliver at S4),
	// not in internal latches here -- a latched memory would deliver a
	// read in the same cycle it was issued.
	uint32_t ReadNow(uint32_t mar) {
		assert(mar * sizeof(uint32_t) < memory_.size());
		uint32_t* ptr = reinterpret_cast<uint32_t*>(memory_.data());
		return ptr[mar];
	}

	void WriteNow(uint32_t mar, uint32_t mdr) {
		assert(mar * sizeof(uint32_t) < memory_.size());
		uint32_t* ptr = reinterpret_cast<uint32_t*>(memory_.data());
		ptr[mar] = mdr;
	}

	// 4-byte block fetch for the instruction fetch unit: same issue/deliver
	// latency model (issue at edge N, delivered at edge N+1). Always reads
	// the full 4 bytes -- a final block may extend past the program into
	// the other segments; only the total-memory bound is guarded.
	void FetchBlock(uint8_t is_fetch, uint32_t imar,
			uint8_t& is_written, uint32_t& block) {
		block = 0;
		is_written = 0;
		if (is_fetch_block_) {
			assert(fetch_block_addr_ + 4 <= memory_.size());
			memcpy(&block, memory_.data() + fetch_block_addr_, 4);
			is_written = 1;
			is_fetch_block_ = 0;
		}
		if (is_fetch) {
			is_fetch_block_ = 1;
			assert(imar + 4 <= memory_.size());
			fetch_block_addr_ = imar;
		}
	}

private:
	uint32_t program_segment_size_ = 0;
	uint32_t constant_segment_size_ = 0;
	uint32_t data_segment_size_ = 0;
	std::vector<uint8_t> memory_;

	uint8_t is_fetch_block_ = 0;
	uint32_t fetch_block_addr_ = 0;
};
