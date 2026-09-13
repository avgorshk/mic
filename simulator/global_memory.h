#pragma once

#include <stdint.h>

#include <cassert>
#include <cstring>
#include <vector>

constexpr uint32_t GLOBAL_MEMORY_SIZE    = 1024 * 1024; // 1 MB
constexpr uint32_t PROGRAM_SEGMENT_SIZE  = 64 * 1024;   // 64 KB
constexpr uint32_t CONSTANT_SEGMENT_SIZE = 64 * 1024;   // 64 KB

class GlobalMemory {
public:
	GlobalMemory()
		: memory_(GLOBAL_MEMORY_SIZE),
		  program_segment_(memory_.data()),
		  constant_segment_(reinterpret_cast<uint32_t*>(program_segment_ + PROGRAM_SEGMENT_SIZE)),
		  stack_segment_(reinterpret_cast<uint32_t*>(program_segment_ + PROGRAM_SEGMENT_SIZE + CONSTANT_SEGMENT_SIZE)) {}

public:
	void SetProgram(const std::vector<uint8_t>& program) {
		assert(program.size() <= PROGRAM_SEGMENT_SIZE);
		memcpy(program_segment_, program.data(), program.size());
	}

private:
	std::vector<uint8_t> memory_;
	uint32_t* constant_segment_;
	uint32_t* stack_segment_;
	uint8_t* program_segment_;
};