#include <iostream>
#include <string>
#include <variant>

#include <base58/base58.hpp>
#include <bignumber/bignumber.hpp>
#include <borsh/borsh.hpp>
#include <borsh/visit_struct.hpp>
#include <ed25519/ed25519.h>
#include <fmt/format.h>
#include <httplib/httplib.h>
#include <ledger.h>
#include <nlohmann/json.hpp>
#include <sha256/sha256.hpp>

#include <near_utils.hpp>

// Based on https://github.com/near-examples/transaction-examples/blob/master/send-tokens-deconstructed.js

class IAction {
};

struct CreateAccount {
};

VISITABLE_EMPTY_STRUCT(CreateAccount);

struct DeployContract {
};

VISITABLE_EMPTY_STRUCT(DeployContract);

struct FunctionCall {
};

VISITABLE_EMPTY_STRUCT(FunctionCall);

struct Transfer {
    BigNumber deposit;
};

std::vector<uint8_t> encode_big_number(const BigNumber& number)
{
    BigNumber number_copy = number;
    uint32_t bytes_required = 1;
    for (; bytes_required <= 64; bytes_required *= 2)
    {
        if (BigNumber(256).pow(bytes_required) > number_copy)
        {
            break;
        }
    }
    std::vector<uint8_t> bytes(bytes_required, 0);
    for (uint32_t byte_index = bytes_required; byte_index > 0; byte_index--)
    {
        BigNumber divisor = BigNumber(256).pow(byte_index);
        if (const BigNumber byte_value = number_copy / divisor; byte_value > 0)
        {
            // Little-endian encoding
            bytes[byte_index - 1] = std::stoi(byte_value.getString());
            number_copy = number_copy - divisor;
        }
    }
    return bytes;
}

ENCODABLE_STRUCT(BigNumber, encode_big_number);

VISITABLE_STRUCT(Transfer, deposit);

struct Stake {
};

VISITABLE_EMPTY_STRUCT(Stake);

struct AddKey {
};

VISITABLE_EMPTY_STRUCT(AddKey);

struct DeleteKey {
};

VISITABLE_EMPTY_STRUCT(DeleteKey);

struct DeleteAccount {
};

VISITABLE_EMPTY_STRUCT(DeleteAccount);

using Action = std::variant<CreateAccount, DeployContract, FunctionCall, Transfer, Stake, AddKey, DeleteKey, DeleteAccount>;

constexpr uint8_t ED25519_KEY_SIZE = 32;

struct ED25519PublicKey {
    std::array<uint8_t, ED25519_KEY_SIZE> data;
};

VISITABLE_STRUCT(ED25519PublicKey, data);

using PublicKey = std::variant<ED25519PublicKey>;

class Transaction {
public:
    std::string signer_id;
    PublicKey public_key;
    uint64_t nonce;
    std::string receiver_id;
    std::array<uint8_t, 32> block_hash;
    std::vector<Action> actions;
};

VISITABLE_STRUCT(Transaction, signer_id, public_key, nonce, receiver_id, block_hash, actions);

int main(int argc, char** argv)
{
    const std::string sender = argv[1];
    const std::string receiver = argv[2];
    const std::string network_id = "testnet";
    const std::string amount = ntb::parse_near_amount("1.5").value_or("");

    if (amount.empty()) {
        std::cerr << fmt::format("Invalid amount '{}'", amount) << std::endl;
        return 1;
    }

    sha256::SHA256 hash;
    hash.update("hello");
    auto digested = hash.digest();
    std::cout << sha256::SHA256::to_string(digested) << std::endl;

    std::string rpc_endpoint = fmt::format("http://rpc.{}.near.org", network_id);

    std::cout << "RPC Endpoint : " << rpc_endpoint << std::endl;

    std::string seed = argv[3];
    std::vector<uint8_t> seed_decoded;
    if (!base58::decode(seed.data(), seed_decoded)) {
        std::cerr << "Invalid base58 NEAR seed" << std::endl;
        return 1;
    }

    std::array<uint8_t, 32> private_key = {};
    std::copy(seed_decoded.begin(), seed_decoded.begin() + 32, private_key.data());
    std::array<uint8_t, 32> public_key = {};
    std::copy(seed_decoded.begin() + 32, seed_decoded.end(), public_key.data());
    std::string public_key_b58 = base58::encode(public_key.data(), public_key.data() + public_key.size());

    std::cout << "Public Key : " << public_key_b58 << std::endl;

    httplib::Client http(rpc_endpoint);

    nlohmann::json payload = {
        { "jsonrpc", "2.0" },
        { "id", "dontcare" },
        { "method", "query" },
        { "params",
            { { "request_type", "view_access_key" },
                { "finality", "final" },
                { "account_id", sender },
                { "public_key", fmt::format("ed25519:{}", public_key_b58) } } }
    };

    auto resp = http.Post("/", payload.dump(), "application/json");
    auto access_key_details = nlohmann::json::parse(resp->body)["result"];

    if (access_key_details["permission"] != "FullAccess") {
        std::cerr << "Account " << sender << " does not have permission to send tokens using key " << public_key_b58 << std::endl;
        return 1;
    }

    uint64_t nonce = access_key_details["nonce"];
    nonce++;

    std::cout << "Using nonce : " << nonce << std::endl;

    std::vector<Action> actions = { Transfer { amount } };

    std::string block_hash_b58 = "HvKszaBzqZohaweUTwy6LBNShKwEUCpR1gZvEGwCVuyK"; // access_key_details["block_hash"]

    std::array<uint8_t, 32> recent_block_hash;
    if (!base58::decode(block_hash_b58, recent_block_hash.data())) {
        std::cerr << fmt::format("Could not decode recent block hash '{}'") << std::endl;
    }
    std::cout << "Block hash : " << block_hash_b58 << std::endl;

    Transaction transaction {
        sender,
        ED25519PublicKey { public_key },
        nonce,
        receiver,
        recent_block_hash,
        actions,
    };

    BorshEncoder encoder;
    encoder.Encode(transaction);

    for (auto c : encoder.GetBuffer()) {
        printf("%d, ", c);
    }
    printf("\n");

    return 0;
}
