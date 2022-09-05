#pragma once

#include <array>
#include <variant>

#include <bignumber/bignumber.hpp>
#include <borsh/visit_struct.hpp>

#include <ntb/encoders.hpp>


namespace ntb::schemas
{
    struct CreateAccount {
    };

    struct DeployContract {
    };

    struct FunctionCall {
        std::string method_name;
        std::vector<uint8_t> args;
        uint64_t gas;
        BigNumber deposit;
    };

    struct Transfer {
        BigNumber deposit;
    };

    struct Stake {
    };

    struct AddKey {
    };

    struct DeleteKey {
    };

    struct DeleteAccount {
    };

    using Action = std::variant<CreateAccount, DeployContract, FunctionCall, Transfer, Stake, AddKey, DeleteKey, DeleteAccount>;

    constexpr uint8_t ED25519_KEY_SIZE = 32;

    struct ED25519PublicKey {
        std::array<uint8_t, ED25519_KEY_SIZE> data;
    };

    using PublicKey = std::variant<ED25519PublicKey>;

    struct Transaction {
        std::string signer_id;
        PublicKey public_key;
        uint64_t nonce;
        std::string receiver_id;
        std::array<uint8_t, 32> block_hash;
        std::vector<Action> actions;
    };

    enum class KeyType
    {
        ED25519
    };

    struct Signature
    {
        KeyType key_type;
        std::array<uint8_t, 64> data;
    };

    struct SignedTransaction
    {
        Transaction transaction;
        Signature signature;
    };
}

VISITABLE_EMPTY_STRUCT(ntb::schemas::CreateAccount);
VISITABLE_EMPTY_STRUCT(ntb::schemas::DeployContract);
VISITABLE_STRUCT(ntb::schemas::FunctionCall, method_name, args, gas, deposit);
VISITABLE_STRUCT(ntb::schemas::Transfer, deposit);
VISITABLE_EMPTY_STRUCT(ntb::schemas::Stake);
VISITABLE_EMPTY_STRUCT(ntb::schemas::AddKey);
VISITABLE_EMPTY_STRUCT(ntb::schemas::DeleteKey);
VISITABLE_EMPTY_STRUCT(ntb::schemas::DeleteAccount);
VISITABLE_STRUCT(ntb::schemas::ED25519PublicKey, data);
VISITABLE_STRUCT(ntb::schemas::Transaction, signer_id, public_key, nonce, receiver_id, block_hash, actions);
VISITABLE_STRUCT(ntb::schemas::Signature, key_type, data);
VISITABLE_STRUCT(ntb::schemas::SignedTransaction, transaction, signature);