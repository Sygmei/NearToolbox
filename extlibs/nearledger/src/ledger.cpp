#include <nearledger/error.hpp>
#include <nearledger/ledger.hpp>
#include <nearledger/utils.hpp>

namespace ledger {
	Ledger::Ledger() : transport_(std::make_unique<Transport>(Transport::TransportType::HID)) {

	}

	Ledger::~Ledger() {
		transport_->close();
	}

	Error Ledger::open() {
		return transport_->open();
	}

	std::tuple<ledger::Error, std::vector<uint8_t>> Ledger::get_version(uint8_t p1, uint8_t p2) {
		std::vector<uint8_t> empty_payload;
		auto [err, buffer] = transport_->exchange(APDU::CLA_NEAR, APDU::INS_GET_APP_CONFIGURATION_NEAR, p1, p2, empty_payload);
		if (err != Error::SUCCESS)
			return {err, {}};
		return {err, std::vector<uint8_t>(buffer.begin(), buffer.end())};
	}

	std::tuple<ledger::Error, std::vector<uint8_t>> Ledger::get_public_key(std::vector<uint8_t> account, uint8_t p1, uint8_t p2) {
		auto [err, buffer] = transport_->exchange(APDU::CLA_NEAR, APDU::INS_GET_PUBLIC_KEY_NEAR, p1, p2, account);
		if (err != Error::SUCCESS)
			return {err, {}};
		return {err, std::vector<uint8_t>(buffer.begin(), buffer.end())};
	}

	std::tuple<Error, std::vector<uint8_t>> Ledger::sign(std::vector<uint8_t> account, const std::vector<uint8_t>& msg, uint8_t p1, uint8_t p2) {
		auto payload = account;
		payload.insert(payload.end(), msg.begin(), msg.end());
		auto [err, buffer] = transport_->exchange(APDU::CLA_NEAR, APDU::INS_SIGN_NEAR, p1, p2, payload);
		if (err != Error::SUCCESS)
			return {err, {}};
		return {err, std::vector<uint8_t>(buffer.begin(), buffer.end())};
	}

	void Ledger::close() {
		return transport_->close();
	}
}
