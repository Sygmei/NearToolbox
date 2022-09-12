#include <iostream>
#include <stdexcept>

#include <base58/base58.hpp>
#include <ed25519/ed25519.h>

extern "C"
{
#include <ed25519/ge.h>
}

#include <ntb/near_utils.hpp>
#include <ntb/signer.hpp>

namespace ntb
{
    ED25519Keypair::ED25519Keypair(const std::string &b58_encoded_private_key_or_seed)
    {
        std::vector<uint8_t> buffer = {};
        const bool result = base58::decode(b58_encoded_private_key_or_seed.data(), buffer);
        if (!result)
        {
            throw std::runtime_error("invalid base58 seed or private key");
        }
        if (buffer.size() == 32)
        {
            // should be a seed
            ed25519_create_keypair(m_public_key.data(), m_private_key.data(), buffer.data());
        }
        else if (buffer.size() == 64)
        {
            // should be a private key or seed + pubkey
            // try as seed first
            std::array<uint8_t, 32> maybe_a_seed = {};
            std::array<uint8_t, 32> maybe_a_public_key = {};
            std::copy(buffer.begin(), buffer.begin() + 32, maybe_a_seed.data());
            std::copy(buffer.begin() + 32, buffer.end(), maybe_a_public_key.data());
            ed25519_create_keypair(m_public_key.data(), m_private_key.data(), maybe_a_seed.data());

            if (maybe_a_public_key != m_public_key)
            {
                // not a seed, storing it as private key instead
                std::copy(buffer.begin(), buffer.end(), m_private_key.data());
                ge_p3 A;
                ge_scalarmult_base(&A, m_private_key.data());
                ge_p3_tobytes(m_public_key.data(), &A);
            }
        }
    }

    ED25519Keypair::ED25519Keypair(const std::array<uint8_t, 64> &private_key)
        : m_private_key(private_key)
    {
        // derive public key from private key
        ge_p3 A;
        ge_scalarmult_base(&A, private_key.data());
        ge_p3_tobytes(m_public_key.data(), &A);
    }

    ED25519Keypair::ED25519Keypair(const std::array<uint8_t, 32> &seed)
    {
        // Building Keypair from seed
        ed25519_create_keypair(m_public_key.data(), m_private_key.data(), seed.data());
    }

    ED25519Keypair ED25519Keypair::create()
    {
        std::array<uint8_t, 32> seed;
        if (ed25519_create_seed(seed.data()) != 0)
        {
            throw std::runtime_error("failed to initialize ED25519 seed");
        }
        return ED25519Keypair(seed);
    }

    std::string ED25519Keypair::get_public_key_as_b58() const
    {
        return base58::encode(m_public_key.data(), m_public_key.data() + m_public_key.size());
    }

    std::array<uint8_t, 32> ED25519Keypair::get_public_key() const
    {
        return m_public_key;
    }

    std::array<uint8_t, 64> ED25519Keypair::sign(const std::vector<uint8_t> &msg) const
    {
        std::array<uint8_t, 64> signature = {};
        ed25519_sign(signature.data(), msg.data(), msg.size(), m_public_key.data(), m_private_key.data());
        return signature;
    }

    std::string ED25519Keypair::get_private_key_as_b58() const
    {
        return base58::encode(m_private_key.data(), m_private_key.data() + m_private_key.size());
    }

    std::array<uint8_t, 64> ED25519Keypair::get_private_key() const
    {
        return m_private_key;
    }

#ifdef NTB_ENABLE_LEDGER
    LedgerWallet::LedgerWallet(const std::string &derivation_path) : m_derivation_path(derivation_path)
    {
        m_ledger.open();
        m_ledger.get_version();
    }

    std::string LedgerWallet::get_public_key_as_b58() const
    {
        const auto public_key = get_public_key();
        return base58::encode(public_key.data(), public_key.data() + public_key.size());
    }

    std::array<uint8_t, 32> LedgerWallet::get_public_key() const
    {
        if (m_public_key)
        {
            return m_public_key.value();
        }
        m_ledger.get_version();
        const std::vector<uint8_t> derivation_path = bip32_path_to_bytes(m_derivation_path);
        const auto public_key_or_error = m_ledger.get_public_key(derivation_path);

        const ledger::Error error = std::get<0>(public_key_or_error);
        if (error != ledger::Error::SUCCESS)
        {
            const std::string error_message = ledger::error_message(error);
            throw std::runtime_error(error_message);
        }
        const std::vector<uint8_t> public_key = std::get<1>(public_key_or_error);
        m_public_key = std::array<uint8_t, 32>{};
        std::copy(public_key.begin(), public_key.end(), m_public_key->data());
        return m_public_key.value();
    }

    std::array<uint8_t, 64> LedgerWallet::sign(const std::vector<uint8_t> &msg) const
    {
        const std::vector<uint8_t> derivation_path = bip32_path_to_bytes(m_derivation_path);
        m_ledger.get_version();
        const auto signed_message_or_error = m_ledger.sign(derivation_path, msg, 0x80);
        const ledger::Error error = std::get<0>(signed_message_or_error);
        if (error != ledger::Error::SUCCESS)
        {
            const std::string error_message = ledger::error_message(error);
            throw std::runtime_error(error_message);
        }
        auto signed_message = std::get<1>(signed_message_or_error);
        std::array<uint8_t, 64> signature = {};
        std::copy(signed_message.begin(), signed_message.end(), signature.data());
        return signature;
    }
#endif
}
