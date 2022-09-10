#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

#include <ntb/near.hpp>

#include "base58/base58.hpp"
#include "base58/base64.hpp"
#include "borsh/borsh.hpp"
#include "ed25519/ed25519.h"
#include "ntb/near_utils.hpp"
#include "sha256/sha256.hpp"

namespace ntb {
NearAmount::operator BigNumber() const {
	return m_amount;
}

NearAmount::NearAmount(int amount) :
		NearAmount(std::to_string(amount)) {
}

NearAmount::NearAmount(long long amount) :
		NearAmount(std::to_string(amount)) {
}

NearAmount::NearAmount(double amount) :
		NearAmount(std::to_string(amount)) {
}

NearAmount::NearAmount(const std::string &amount) :
		m_amount(ntb::parse_near_amount(amount).value_or("0")) {
}

NearAmount::NearAmount(const char *amount) :
		NearAmount(std::string(amount)) {
}

std::string NearClient::_get_rpc_endpoint(const std::string &network) {
	return fmt::format("http://rpc.{}.near.org", network);
}

void NearClient::_load_access_key() {
	// Retrieving AccessKey details
	const nlohmann::json access_key_query_parameters = {
		{ "request_type", "view_access_key" },
		{ "finality", "final" },
		{ "account_id", m_account_id },
		{ "public_key", fmt::format("ed25519:{}", m_signer->get_public_key_as_b58()) }
	};

	auto access_key_resp = m_rpc.query(access_key_query_parameters);
	auto access_key_result = access_key_resp.expect("failed to get AccessKey details").data;

	std::cout << "PublicKey " << m_signer->get_public_key_as_b58() << std::endl;
	std::cout << "AccessKey " << access_key_result.dump() << std::endl;

	const std::string permission_level = access_key_result["permission"];
	const uint64_t nonce = access_key_result["nonce"];
	m_access_key = AccessKey{
		magic_enum::enum_cast<AccessKeyPermission>(permission_level).value_or(AccessKeyPermission::None),
		nonce,
		access_key_result["block_hash"]
	};
}

std::string NearClient::account_id_from_public_key(const std::array<uint8_t, 32> &public_key) {
	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (const uint8_t byte : public_key) {
		ss << std::hex << std::setw(2) << static_cast<int>(byte);
	}

	return ss.str();
}

std::string NearClient::account_id_from_online_resolver([[maybe_unused]] const std::string &resolver_url,
		[[maybe_unused]] const std::array<uint8_t, 32> &public_key) {
	return ""; // TODO
}

void NearClient::_assert_access_key_sufficient_permissions(AccessKeyPermission minimum_permission) {
	const bool sufficient_permission = static_cast<uint8_t>(m_access_key.permission) >= static_cast<uint8_t>(minimum_permission);
	if (!sufficient_permission) {
		throw std::runtime_error("AccessKey has unsufficient permission");
	}
}

void NearClient::_resolve_account_id(AccountId account_id) {
	if (std::holds_alternative<ImplicitAccount>(account_id)) {
		m_account_id = NearClient::account_id_from_public_key(m_signer->get_public_key());
	} else if (std::holds_alternative<NamedAccount>(account_id)) {
		m_account_id = std::get<NamedAccount>(account_id).account_id;
	} else if (std::holds_alternative<AccountIdResolver>(account_id)) {
		const std::string resolver_url = std::get<AccountIdResolver>(account_id).resolver_url;
		m_account_id = account_id_from_online_resolver(resolver_url, m_signer->get_public_key());
	}
}

TransactionResult NearClient::transaction(const std::string &recipient, const std::vector<schemas::Action> &actions) {
	_load_access_key();
	_assert_access_key_sufficient_permissions(AccessKeyPermission::FullAccess);

	uint64_t nonce = m_access_key.nonce + (++m_nonce_increment);

	// Retrieving recent block hash
	std::string block_hash_b58 = m_access_key.block_hash;
	std::array<uint8_t, 32> recent_block_hash = {};
	if (!base58::decode(block_hash_b58, recent_block_hash.data())) {
		throw std::runtime_error(fmt::format("Could not decode recent block hash '{}'", block_hash_b58));
	}

	// Building Transaction struct
	ntb::schemas::Transaction transaction{
		m_account_id,
		ntb::schemas::ED25519PublicKey{ m_signer->get_public_key() },
		nonce,
		recipient,
		recent_block_hash,
		actions,
	};

	// Encoding Transaction using Borsh
	BorshEncoder tx_encoder;
	tx_encoder.Encode(transaction);
	std::vector<uint8_t> tx_bytes = tx_encoder.GetBuffer();
	std::cout << "TX PAYLOAD [";
	for (uint8_t byte : tx_bytes) {
		std::cout << static_cast<int>(byte) << ", ";
	}
	std::cout << "]" << std::endl;

	// Building Transaction signature
	sha256::SHA256 tx_hasher;
	tx_hasher.update(tx_bytes.data(), tx_bytes.size());
	sha256::hash_container tx_hash = tx_hasher.digest();
	std::vector<uint8_t> msg_to_sign = std::vector<uint8_t>(tx_hash.begin(), tx_hash.end());
	std::array<uint8_t, 64> signature = m_signer->sign(msg_to_sign);

	// Building SignedTransaction
	ntb::schemas::SignedTransaction signed_transaction{ transaction, ntb::schemas::Signature{ ntb::schemas::KeyType::ED25519, signature } };
	BorshEncoder signed_tx_encoder;
	signed_tx_encoder.Encode(signed_transaction);
	std::vector<uint8_t> signed_tx_bytes = signed_tx_encoder.GetBuffer();

	// Broadcasting transaction
	nlohmann::json broadcast_tx_parameters = { base64::encode(signed_tx_bytes.data(), signed_tx_bytes.size()) };
	auto broadcast_resp = m_rpc.call("broadcast_tx_commit", broadcast_tx_parameters);
	auto broadcast_result = broadcast_resp.expect("failed to broadcast transaction").data; // TODO: better error handling

	return TransactionResult{ broadcast_result };
}

TransactionResult NearClient::transfer(const std::string &recipient, const NearAmount &amount) {
	return transaction(recipient, { ntb::schemas::Transfer{ amount } });
}

constexpr uint64_t MAX_GAS = 30000000000000;

ContractCallResult NearClient::contract_call(const std::string &contract_address, const std::string &method_name,
		const nlohmann::json &parameters, const NearAmount &deposit) {
	const std::string parameters_b64 = parameters.dump();
	const std::vector<uint8_t> parameters_bytes(parameters_b64.cbegin(), parameters_b64.cend());
	const std::vector<schemas::Action> actions = { schemas::FunctionCall{ method_name, parameters_bytes, MAX_GAS, deposit } };
	const auto tx_data = transaction(contract_address, actions);
	return ContractCallResult{ tx_data, {} };
}

ContractCallResult NearClient::contract_view(const std::string &contract_address, const std::string &method_name,
		const nlohmann::json &parameters) {
	const std::string call_parameters_b64 = base64::encode(parameters.dump());
	const nlohmann::json query_parameters = {
		{ "request_type", "call_function" },
		{ "finality", "final" },
		{ "account_id", contract_address },
		{ "method_name", method_name },
		{ "args_base64", call_parameters_b64 }
	};
	auto query_resp = m_rpc.query(query_parameters);
	auto tx_data = query_resp.expect("failed to query smart contract").data;
	std::vector<uint8_t> call_result_bytes = tx_data["result"];
	const std::string call_result_str = std::string(call_result_bytes.begin(), call_result_bytes.end());
	return ContractCallResult{ tx_data, nlohmann::json::parse(call_result_str) };
}
} // namespace ntb
