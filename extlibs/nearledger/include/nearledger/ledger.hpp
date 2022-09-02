#pragma once

#include <cstdint>
#include <tuple>
#include <vector>

#include <nearledger/transport.hpp>

namespace ledger {
	class Ledger {
		enum APDU : uint8_t {
			CLA = 0xE0,
			CLA_NEAR = 0x80,
			INS_GET_APP_CONFIGURATION = 0x01,
			INS_GET_PUBLIC_KEY = 0x02,
			INS_SIGN = 0x03,
			INS_GET_APP_CONFIGURATION_NEAR = 0x06,
			INS_GET_PUBLIC_KEY_NEAR = 0x04,
			INS_SIGN_NEAR = 0x02,
		};

	public:
		Ledger();
		~Ledger();

		Error open();

		std::tuple<Error, std::vector<uint8_t>> get_version(uint8_t p1 = 0x00, uint8_t p2 = 0x00);
		std::tuple<Error, std::vector<uint8_t>> get_public_key(std::vector<uint8_t> account, uint8_t p1 = 0x00, uint8_t p2 = 0x00);
		std::tuple<Error, std::vector<uint8_t>> sign(std::vector<uint8_t> account, const std::vector<uint8_t>& msg, uint8_t p1 = 0x00, uint8_t p2 = 0x00);

		void close();

	private:
		std::unique_ptr<Transport> transport_;
	};
}
