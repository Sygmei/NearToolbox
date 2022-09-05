#pragma once

#include <array>
#include <string_view>

#include <bignumber/bignumber.hpp>
#include <nlohmann/json.hpp>

#include <ntb/rpc.hpp>
#include <ntb/schemas.hpp>



namespace ntb
{
    namespace networks
    {
        constexpr std::string_view testnet = "testnet";
        constexpr std::string_view mainnet = "mainnet";
    }

    struct ED25519Seed
    {
        std::array<uint8_t, 32> seed;

        ED25519Seed(const std::string& b58_encoded_seed);
        ED25519Seed(const std::array<uint8_t, 32>& seed);
    };

    struct ED25519Keypair
    {
        std::array<uint8_t, 64> private_key;
        std::array<uint8_t, 32> public_key;

        ED25519Keypair(const std::string& b58_encoded_private_key);
        ED25519Keypair(const std::array<uint8_t, 64>& private_key);
    };

    class TransactionResult
    {
        
    };

    class TransferResult
    {
        
    };

    struct NearAmount
    {
        BigNumber amount;

        NearAmount(double amount);
        NearAmount(const std::string& amount);
        NearAmount(const char* amount);

        static NearAmount from_yocto(const std::string& amount);
    };

    constexpr std::string_view NEAR_DERIVATION_PATH = "44'/397'/0'/0'/1'";

    class NearClient
    {
    private:
        std::string m_network;
        ntb::RPCClient m_rpc;

        bool m_hardware_wallet = false;
        std::string m_derivation_path;

        std::string m_account_id;
        ED25519Keypair m_keypair;
    protected:
        void _check_access_key_permissions();
        void _resolve_account_id(); // see: https://github.com/near/near-indexer-for-explorer#shared-public-access
                                    // with: https://github.com/taocpp/taopq
    public:
        NearClient(const std::string_view network);

        static ED25519Keypair create_account();

        void login(const ED25519Seed& seed, const std::string& account_id = "");
        void login(const ED25519Keypair& keypair, const std::string& account_id = "");
        void login(const std::string& seed_or_private_key, const std::string& account_id = "");
        void login_with_ledger(std::string_view derivation_path = NEAR_DERIVATION_PATH, const std::string& account_id = "");

        TransactionResult transaction(const std::string& recipient, const std::vector<schemas::Action>& actions);
        TransferResult transfer(const std::string& recipient, const NearAmount& amount);
        void contract_view(const std::string& contract_address, const std::string& method_name, const nlohmann::json& parameters);
        void contract_call(const std::string& contract_address, const std::string& method_name, const nlohmann::json& parameters);
    };
}
