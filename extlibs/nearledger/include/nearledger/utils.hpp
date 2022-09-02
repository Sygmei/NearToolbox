#pragma once

#include <cstdint>
#include <vector>

namespace ledger::utils {
	int bytes_to_int(const std::vector<uint8_t>& bytes);
	std::vector<uint8_t> int_to_bytes(unsigned int n, unsigned int length);
} // namespace ledger::utils
