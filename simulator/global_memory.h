#pragma once

#include <stdint.h>

#include <vector>

constexpr uint32_t GLOBAL_MEMORY_SIZE    = 1024 * 1024; // 1 MB
constexpr uint32_t PROGRAM_SEGMENT_SIZE  = 64 * 1024;   // 64 KB
constexpr uint32_t CONSTANT_SEGMENT_SIZE = 64 * 1024;   // 64 KB

class GlobalMemory {
public:
	GlobalMemory()
		: memory_(GLOBAL_MEMORY_SIZE),
		  program_segment_(memory_.data()),
		  constant_segment_(reinterpret_cast<const uint32_t*>(program_segment_ + PROGRAM_SEGMENT_SIZE)),
		  stack_segment_(reinterpret_cast<const uint32_t*>(program_segment_ + PROGRAM_SEGMENT_SIZE + CONSTANT_SEGMENT_SIZE)) {}

private:
	std::vector<uint8_t> memory_;
	const uint32_t* constant_segment_;
	const uint32_t* stack_segment_;
	const uint8_t* program_segment_;
};