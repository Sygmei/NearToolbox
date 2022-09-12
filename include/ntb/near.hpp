#pragma once

#include <array>
#include <atomic>
#include <string_view>

#include <bignumber/bignumber.hpp>
#include <nlohmann/json.hpp>

#include <ntb/rpc.hpp>
#include <ntb/schemas.hpp>
#include <ntb/signer.hpp>

namespace ntb
{
	namespace networks
	{
		constexpr std::string_view testnet = "testnet";
		constexpr std::string_view mainnet = "mainnet";
	} // namespace networks

	struct TransactionResult
	{
		nlohmann::json tx_data;
	};

	struct ContractCallResult : public TransactionResult
	{
		nlohmann::json result;
	};

	class TransferResult
	{
	};

	enum class AccessKeyPermission
	{
		None,
		FunctionCall,
		FullAccess,
	};

	struct AccessKey
	{
		AccessKeyPermission permission;
		uint64_t nonce;
		std::string block_hash;
	};

	struct NearAmount
	{
	protected:
		BigNumber m_amount;

	public:
		NearAmount(int amount);
		NearAmount(long long amount);
		NearAmount(double amount);
		NearAmount(const std::string &amount);
		NearAmount(const char *amount);

		static NearAmount from_yocto(const std::string &amount)
		{
			NearAmount near_amount(0.0);
			near_amount.m_amount = amount;
			return near_amount;
		}

		operator BigNumber() const;
	};

	struct ImplicitAccount
	{
	};

	struct NamedAccount
	{
		std::string account_id;
	};

	struct AccountIdResolver
	{
		std::string resolver_url = "http://resolver.near.org/network/{}/public_key/{}";
	};

	using AccountId = std::variant<ImplicitAccount, NamedAccount, AccountIdResolver>;

	class NearClient
	{
	private:
		std::string m_network;
		ntb::RPCClient m_rpc;

		std::string m_account_id;
		std::unique_ptr<Signer> m_signer;
		AccessKey m_access_key;
		std::atomic<int> m_nonce_increment;

	protected:
		static std::string _get_rpc_endpoint(const std::string &network);
		void _load_access_key();
		void _assert_access_key_sufficient_permissions(AccessKeyPermission minimum_permission);
		void _resolve_account_id(AccountId account_id);

	public:
		template <class SignerClass>
		explicit NearClient(const std::string_view network, SignerClass &&signing_method, const AccountId &account_id);

		TransactionResult transaction(const std::string &recipient, const std::vector<schemas::Action> &actions);
		TransactionResult transfer(const std::string &recipient, const NearAmount &amount);
		ContractCallResult contract_view(const std::string &contract_address, const std::string &method_name, const nlohmann::json &parameters = nlohmann::json::value_t::object);
		ContractCallResult contract_call(const std::string &contract_address, const std::string &method_name, const nlohmann::json &parameters = nlohmann::json::value_t::object, const NearAmount &deposit = 0);

		static std::string account_id_from_public_key(const std::array<uint8_t, 32> &public_key);
		static std::string account_id_from_online_resolver(const std::string &resolver_url, const std::array<uint8_t, 32> &public_key); // see: https://github.com/near/near-indexer-for-explorer#shared-public-access// with: https://github.com/taocpp/taopq
	};

	template <class SignerClass>
	NearClient::NearClient(const std::string_view network, SignerClass &&signing_method, const AccountId &account_id)
		: m_network(network), m_rpc(_get_rpc_endpoint(network.data())), m_signer(std::make_unique<SignerClass>(std::move(signing_method)))
	{
		_resolve_account_id(account_id);
		_load_access_key();
	}
}
