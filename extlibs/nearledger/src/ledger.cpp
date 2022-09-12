#include <nearledger/error.hpp>
#include <nearledger/ledger.hpp>
#include <nearledger/utils.hpp>
#include <utility>
#include <iostream>

namespace ledger
{
	Ledger::Ledger() : transport_(std::make_unique<Transport>(Transport::TransportType::HID))
	{
	}

	Ledger::Ledger(Ledger &&other) : transport_(other.transport_.release())
	{
	}

	Ledger::~Ledger()
	{
		if (transport_)
		{
			transport_->close();
		}
	}

	Error Ledger::open()
	{
		return transport_->open();
	}

	std::tuple<ledger::Error, std::vector<uint8_t>> Ledger::get_version(uint8_t p1, uint8_t p2) const
	{
		const std::vector<uint8_t> empty_payload;
		auto [err, buffer] = transport_->exchange(APDU::CLA_NEAR, APDU::INS_GET_APP_CONFIGURATION_NEAR, p1, p2, empty_payload);
		if (err != Error::SUCCESS)
			return {err, {}};
		return {err, std::vector<uint8_t>(buffer.begin(), buffer.end())};
	}

	std::tuple<ledger::Error, std::vector<uint8_t>> Ledger::get_public_key(std::vector<uint8_t> account, uint8_t p1, uint8_t p2) const
	{
		auto [err, buffer] = transport_->exchange(APDU::CLA_NEAR, APDU::INS_GET_PUBLIC_KEY_NEAR, p1, p2, account);
		if (err != Error::SUCCESS)
			return {err, {}};
		return {err, std::vector<uint8_t>(buffer.begin(), buffer.end())};
	}

	std::tuple<Error, std::vector<uint8_t>> Ledger::sign(std::vector<uint8_t> account, const std::vector<uint8_t> &msg, uint8_t p1, uint8_t p2) const
	{
		std::cout << "Signing " << std::endl;
		std::cout << "  Account" << std::endl;
		std::cout << "    ";
		for (uint8_t byte : account)
		{
			std::cout << " " << static_cast<int>(byte);
		}
		std::cout << std::endl;
		std::cout << "  Msg" << std::endl;
		std::cout << "    ";
		for (uint8_t byte : msg)
		{
			std::cout << " " << static_cast<int>(byte);
		}
		std::cout << std::endl;
		auto payload = std::move(account);
		payload.insert(payload.end(), msg.begin(), msg.end());
		auto [err, buffer] = transport_->exchange(APDU::CLA_NEAR, APDU::INS_SIGN_NEAR, p1, p2, payload);
		std::cout << "  Error " << ledger::error_message(err) << std::endl;
		std::cout << "  Signed " << buffer.size() << std::endl;
		std::cout << "    ";
		for (uint8_t byte : buffer)
		{
			std::cout << " " << static_cast<int>(byte);
		}
		std::cout << std::endl;
		std::cout << "DONE SIGNING WITH LEDGER" << std::endl;
		if (err != Error::SUCCESS)
			return {err, {}};
		return {err, std::vector<uint8_t>(buffer.begin(), buffer.end())};
	}

	void Ledger::close() const
	{
		if (transport_)
		{
			transport_->close();
		}
	}
}
