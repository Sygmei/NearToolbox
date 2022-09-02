#pragma once

#include <vector>

#include <nearledger/error.hpp>

namespace ledger {
	class Comm {
	public:
		virtual ~Comm() = default;

		virtual Error open() = 0;
		virtual int send(const std::vector<uint8_t>& data) = 0;
		virtual int recv(std::vector<uint8_t>& rdata) = 0;
		virtual void close() = 0;
		[[nodiscard]] virtual bool is_open() const = 0;
	};
} // namespace ledger
