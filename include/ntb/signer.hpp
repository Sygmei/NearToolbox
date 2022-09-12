#pragma once

#ifdef NTB_ENABLE_LEDGER
#include <optional>
#endif

#include <array>
#include <string>
#include <vector>

#include <nearledger/ledger.hpp>

// #define NTB_ENABLE_LEDGER

namespace ntb
{
    class Signer
    {
    public:
        virtual ~Signer() = default;

        [[nodiscard]] virtual std::string get_public_key_as_b58() const = 0;
        [[nodiscard]] virtual std::array<uint8_t, 32> get_public_key() const = 0;
        [[nodiscard]] virtual std::array<uint8_t, 64> sign(const std::vector<uint8_t> &msg) const = 0;
    };

    class ED25519Keypair : public Signer
    {
    private:
        std::array<uint8_t, 64> m_private_key;
        std::array<uint8_t, 32> m_public_key;

    public:
        ED25519Keypair(const std::string &b58_encoded_private_key_or_seed);
        ED25519Keypair(const std::array<uint8_t, 64> &private_key);
        ED25519Keypair(const std::array<uint8_t, 32> &seed);

        static ED25519Keypair create();

        [[nodiscard]] std::string get_public_key_as_b58() const override;
        [[nodiscard]] std::array<uint8_t, 32> get_public_key() const override;
        [[nodiscard]] std::array<uint8_t, 64> sign(const std::vector<uint8_t> &msg) const override;

        [[nodiscard]] std::string get_private_key_as_b58() const;
        [[nodiscard]] std::array<uint8_t, 64> get_private_key() const;
    };

    constexpr std::string_view NEAR_DERIVATION_PATH = "44'/397'/0'/0'/1'";

#ifdef NTB_ENABLE_LEDGER
    struct LedgerWallet : public Signer
    {
    private:
        std::string m_derivation_path;
        mutable std::optional<std::array<uint8_t, 32>> m_public_key;
        ledger::Ledger m_ledger;

    public:
        LedgerWallet(const std::string &derivation_path = NEAR_DERIVATION_PATH.data());

        [[nodiscard]] std::string get_public_key_as_b58() const override;
        [[nodiscard]] std::array<uint8_t, 32> get_public_key() const override;
        [[nodiscard]] std::array<uint8_t, 64> sign(const std::vector<uint8_t> &msg) const override;
    };
#endif
}
