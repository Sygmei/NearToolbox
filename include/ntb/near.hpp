#pragma once

#include <array>
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
    }

    struct TransactionResult
    {
        nlohmann::json data;
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
        NearAmount(double amount);
        NearAmount(const std::string &amount);
        NearAmount(const char *amount);

        friend static NearAmount from_yocto(const std::string &amount)
        {
            NearAmount near_amount(0.0);
            near_amount.m_amount = amount;
            return near_amount;
        }

        operator BigNumber() const;
    };

    constexpr std::string_view NEAR_DERIVATION_PATH = "44'/397'/0'/0'/1'";

    class NearClient
    {
    private:
        std::string m_network;
        ntb::RPCClient m_rpc;

        std::string m_account_id;
        std::unique_ptr<Signer> m_signer;
        AccessKey m_access_key;

    protected:
        static std::string _get_rpc_endpoint(const std::string& network);
        void _load_access_key();
        void _resolve_account_id(); // see: https://github.com/near/near-indexer-for-explorer#shared-public-access
                                    // with: https://github.com/taocpp/taopq
        void _assert_access_key_sufficient_permissions(AccessKeyPermission minimum_permission);

    public:
        template <class SignerClass>
        explicit NearClient(const std::string_view network, const SignerClass& signing_method, const std::string& account_id = "");

        TransactionResult transaction(const std::string &recipient, const std::vector<schemas::Action> &actions);
        TransactionResult transfer(const std::string &recipient, const NearAmount &amount);
        void contract_view(const std::string &contract_address, const std::string &method_name, const nlohmann::json &parameters);
        void contract_call(const std::string &contract_address, const std::string &method_name, const nlohmann::json &parameters);
    };

    template <class SignerClass>
    NearClient::NearClient(const std::string_view network, const SignerClass& signing_method, const std::string& account_id)
        : m_network(network)
        , m_rpc(_get_rpc_endpoint(network.data()))
        , m_signer(std::make_unique<SignerClass>(signing_method))
        , m_account_id(account_id)
    {
        _load_access_key();
    }
}
