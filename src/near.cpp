#include <base58/base58.hpp>
#include <ed25519/ed25519.h>
#include <ed25519/ge.h>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

#include <ntb/near.hpp>

namespace ntb
{
    ED25519Keypair::ED25519Keypair(const std::string &b58_encoded_private_key_or_seed)
    {
        std::vector<uint8_t> buffer = {};
        bool result = base58::decode(b58_encoded_private_key_or_seed.data(), buffer.data());
        if (!result)
        {
            throw std::runtime_error("invalid base58 seed or private key");
        }
        if (buffer.size() == 32)
        {
            // should be a seed
            ed25519_create_keypair(public_key.data(), private_key.data(), buffer.data());
        }
        else if (buffer.size() == 64)
        {
            // should be a private key
            std::copy(buffer.begin(), buffer.end(), private_key);
            ge_p3 A;
            ge_scalarmult_base(&A, private_key.data());
            ge_p3_tobytes(public_key.data(), &A);
        }
    }

    ED25519Keypair::ED25519Keypair(const std::array<uint8_t, 64> &private_key) : private_key(private_key)
    {
        // derive public key from private key
        ge_p3 A;
        ge_scalarmult_base(&A, private_key.data());
        ge_p3_tobytes(public_key.data(), &A);
    }

    ED25519Keypair::ED25519Keypair(const std::array<uint8_t, 32> &seed)
    {
        // Building Keypair from seed
        ed25519_create_keypair(public_key.data(), private_key.data(), seed.data());
    }

    std::string ED25519Keypair::private_key_as_b58() const
    {
        return base58::encode(private_key.data(), private_key.data() + private_key.size());
    }

    std::string ED25519Keypair::public_key_as_b58() const
    {
        return base58::encode(public_key.data(), public_key.data() + public_key.size());
    }

    void NearClient::_load_access_key()
    {
        // Retrieving AccessKey details
        nlohmann::json access_key_query_parameters = {
            {"request_type", "view_access_key"},
            {"finality", "final"},
            {"account_id", m_account_id},
            {"public_key", fmt::format("ed25519:{}", m_keypair.public_key_as_b58())}};

        auto access_key_resp = m_rpc.query(access_key_query_parameters);
        auto access_key_result = access_key_resp.expect("failed to get AccessKey details").data;

        m_access_key = AccessKey{
            magic_enum::enum_cast<AccessKeyPermission>(access_key_result["permission"]).value_or(AccessKeyPermission::None),
            access_key_result["nonce"],
            access_key_result["result"]};
    }

    void NearClient::_assert_access_key_sufficient_permissions(AccessKeyPermission minimum_permission)
    {
        return static_cast<uint8_t>(m_access_key.permission) > static_cast<uint8_t>(minimum_permission);
    }
}
