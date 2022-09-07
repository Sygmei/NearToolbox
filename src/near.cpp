#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

#include <ntb/near.hpp>

#include "base58/base58.hpp"
#include "base58/base64.hpp"
#include "borsh/borsh.hpp"
#include "ed25519/ed25519.h"
#include "ntb/near_utils.hpp"
#include "sha256/sha256.hpp"

namespace ntb
{
    NearAmount::operator BigNumber() const
    {
        return m_amount;
    }

    NearAmount::NearAmount(double amount) : NearAmount(std::to_string(amount))
    {
    }

    NearAmount::NearAmount(const std::string& amount) : m_amount(ntb::parse_near_amount(amount).value_or("0"))
    {
    }

    NearAmount::NearAmount(const char* amount) : NearAmount(std::string(amount))
    {
    }


    std::string NearClient::_get_rpc_endpoint(const std::string& network)
    {
        return fmt::format("http://rpc.{}.near.org", network);
    }

    void NearClient::_load_access_key()
    {
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
        m_access_key = AccessKey {
            magic_enum::enum_cast<AccessKeyPermission>(permission_level).value_or(AccessKeyPermission::None),
            nonce,
            access_key_result["block_hash"]
        };
    }

    void NearClient::_assert_access_key_sufficient_permissions(AccessKeyPermission minimum_permission)
    {
        const bool sufficient_permission = static_cast<uint8_t>(m_access_key.permission) >= static_cast<uint8_t>(minimum_permission);
        if (!sufficient_permission) {
            throw std::runtime_error("AccessKey has unsufficient permission");
        }
    }

    TransactionResult NearClient::transaction(const std::string& recipient, const std::vector<schemas::Action>& actions)
    {
        _assert_access_key_sufficient_permissions(AccessKeyPermission::FullAccess);

        uint64_t nonce = m_access_key.nonce;
        nonce++;

        // Retrieving recent block hash
        std::string block_hash_b58 = m_access_key.block_hash;
        std::array<uint8_t, 32> recent_block_hash;
        if (!base58::decode(block_hash_b58, recent_block_hash.data()))
        {
            throw std::runtime_error(fmt::format("Could not decode recent block hash '{}'", block_hash_b58));
        }

        // Building Transaction struct
        ntb::schemas::Transaction transaction{
            m_account_id,
            ntb::schemas::ED25519PublicKey{m_signer->get_public_key()},
            nonce,
            recipient,
            recent_block_hash,
            actions,
        };

        // Encoding Transaction using Borsh
        BorshEncoder tx_encoder;
        tx_encoder.Encode(transaction);
        std::vector<uint8_t> tx_bytes = tx_encoder.GetBuffer();

        // Building Transaction signature
        sha256::SHA256 tx_hasher;
        tx_hasher.update(tx_bytes.data(), tx_bytes.size());
        sha256::hash_container tx_hash = tx_hasher.digest();
        std::vector<uint8_t> msg_to_sign = std::vector<uint8_t>(tx_hash.begin(), tx_hash.end());
        std::array<uint8_t, 64> signature = m_signer->sign(msg_to_sign);

        // Building SignedTransaction
        ntb::schemas::SignedTransaction signed_transaction{ transaction, ntb::schemas::Signature{ntb::schemas::KeyType::ED25519, signature} };
        BorshEncoder signed_tx_encoder;
        signed_tx_encoder.Encode(signed_transaction);
        std::vector<uint8_t> signed_tx_bytes = signed_tx_encoder.GetBuffer();

        // Broadcasting transaction
        nlohmann::json broadcast_tx_parameters = { base64::encode(signed_tx_bytes.data(), signed_tx_bytes.size()) };
        auto broadcast_resp = m_rpc.call("broadcast_tx_commit", broadcast_tx_parameters);
        auto broadcast_result = broadcast_resp.expect("failed to broadcast transaction").data;

        return TransactionResult { broadcast_result };
    }

    TransactionResult NearClient::transfer(const std::string& recipient, const NearAmount& amount)
    {
        return transaction(recipient, { ntb::schemas::Transfer{amount} });
    }
}
