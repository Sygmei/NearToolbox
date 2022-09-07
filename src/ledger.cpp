#include <iostream>
#include <string>
#include <variant>

#include <base58/base58.hpp>
#include <base58/base64.hpp>
#include <bignumber/bignumber.hpp>
#include <borsh/borsh.hpp>
#include <borsh/visit_struct.hpp>
#include <ed25519/ed25519.h>
#include <fmt/format.h>
#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <sha256/sha256.hpp>

#include <ntb/near.hpp>
#include <ntb/near_utils.hpp>
#include <ntb/rpc.hpp>
#include <ntb/schemas.hpp>

// Based on https://github.com/near-examples/transaction-examples/blob/master/send-tokens-deconstructed.js

int tx_example([[maybe_unused]] int argc, char** argv)
{
    const std::string sender = argv[1];
    const std::string receiver = argv[2];
    const std::string network_id = "testnet";
    const std::string amount = ntb::parse_near_amount("1.24").value_or("");

    if (amount.empty()) {
        std::cerr << fmt::format("Invalid amount '{}'", amount) << std::endl;
        return 1;
    }

    // Building RPC client
    std::string rpc_endpoint = fmt::format("http://rpc.{}.near.org", network_id);
    std::cout << "RPC Endpoint : " << rpc_endpoint << std::endl;
    ntb::RPCClient rpc_client(rpc_endpoint);

    // Decoding seed
    std::string ed25519_seed_b58 = argv[3];
    std::array<uint8_t, 32> ed25519_seed;
    if (!base58::decode(ed25519_seed_b58.data(), ed25519_seed.data())) {
        std::cerr << "Invalid base58 NEAR seed" << std::endl;
        return 1;
    }

    // Building Keypair from seed
    std::array<uint8_t, 32> public_key = {};
    std::array<uint8_t, 64> private_key = {};
    ed25519_create_keypair(public_key.data(), private_key.data(), ed25519_seed.data());
    std::string public_key_b58 = base58::encode(public_key.data(), public_key.data() + public_key.size());
    std::cout << "Public Key : " << public_key_b58 << std::endl;

    // Retrieving AccessKey details
    nlohmann::json access_key_query_parameters = {
        { "request_type", "view_access_key" },
        { "finality", "final" },
        { "account_id", sender },
        { "public_key", fmt::format("ed25519:{}", public_key_b58) }
    };
    auto access_key_resp = rpc_client.query(access_key_query_parameters);
    auto access_key_result = access_key_resp.expect("failed to get AccessKey details").data;

    std::cout << "AccessKey details : " << access_key_result.dump() << std::endl;

    // Asserting AccessKey has sufficient permissions
    if (access_key_result["permission"] != "FullAccess") {
        std::cerr << "Account " << sender << " does not have permission to send tokens using key " << public_key_b58 << std::endl;
        return 1;
    }

    // Using next nonce value
    uint64_t nonce = access_key_result["nonce"];
    nonce++;
    std::cout << "Using nonce : " << nonce << std::endl;

    // Single Transfer action
    std::vector<ntb::schemas::Action> actions = { ntb::schemas::Transfer { amount } };

    // Retrieving recent block hash
    std::string block_hash_b58 = access_key_result["block_hash"];
    std::array<uint8_t, 32> recent_block_hash;
    if (!base58::decode(block_hash_b58, recent_block_hash.data())) {
        std::cerr << fmt::format("Could not decode recent block hash '{}'", block_hash_b58) << std::endl;
    }
    std::cout << "Block hash : " << block_hash_b58 << std::endl;

    // Building Transaction struct
    ntb::schemas::Transaction transaction {
        sender,
        ntb::schemas::ED25519PublicKey { public_key },
        nonce,
        receiver,
        recent_block_hash,
        actions,
    };

    // Encoding Transaction using Borsh
    BorshEncoder tx_encoder;
    tx_encoder.Encode(transaction);
    std::cout << std::endl;
    std::cout << "Final bytes : ";
    for (auto c : tx_encoder.GetBuffer()) {
        printf("%d, ", c);
    }
    printf("\n");
    std::vector<uint8_t> tx_bytes = tx_encoder.GetBuffer();

    // Building Transaction signature
    sha256::SHA256 tx_hasher;
    tx_hasher.update(tx_bytes.data(), tx_bytes.size());
    sha256::hash_container tx_hash = tx_hasher.digest();
    std::array<uint8_t, 64> signature = {};
    std::cout << "Message to sign : " << base58::encode(tx_hash.data(), tx_hash.data() + tx_hash.size()) << std::endl;
    ed25519_sign(signature.data(), tx_hash.data(), tx_hash.size(), public_key.data(), private_key.data());

    // Building SignedTransaction
    ntb::schemas::SignedTransaction signed_transaction { transaction, ntb::schemas::Signature { ntb::schemas::KeyType::ED25519, signature } };
    BorshEncoder signed_tx_encoder;
    signed_tx_encoder.Encode(signed_transaction);
    std::vector<uint8_t> signed_tx_bytes = signed_tx_encoder.GetBuffer();

    // Broadcasting transaction
    std::cout << "Broadcasting transaction..." << std::endl;
    nlohmann::json broadcast_tx_parameters = { base64::encode(signed_tx_bytes.data(), signed_tx_bytes.size()) };
    auto broadcast_resp = rpc_client.call("broadcast_tx_commit", broadcast_tx_parameters);
    auto broadcast_result = broadcast_resp.expect("failed to broadcast transaction").data;
    std::cout << "Tx result " << broadcast_result.dump();
    std::cout << "Transaction broadcasted at " << broadcast_result["transaction"]["hash"] << std::endl;

    return 0;
}

int tx_example_simple([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    const std::string sender = argv[1];
    const std::string receiver = argv[2];
    const std::string network_id = "testnet";

    ntb::NearClient near_client(network_id, ntb::ED25519Keypair(argv[3]), sender);
    near_client.transfer(receiver, "0.84");

    return 0;
}

constexpr uint64_t MAX_GAS = 30000000000000;

int smart_contract_call_example([[maybe_unused]] int argc, char** argv)
{
    const std::string sender = argv[1];
    const std::string receiver = argv[2];
    const std::string network_id = "testnet";
    const std::string amount = ntb::parse_near_amount("1.24").value_or("");

    if (amount.empty()) {
        std::cerr << fmt::format("Invalid amount '{}'", amount) << std::endl;
        return 1;
    }

    // Building RPC client
    std::string rpc_endpoint = fmt::format("http://rpc.{}.near.org", network_id);
    std::cout << "RPC Endpoint : " << rpc_endpoint << std::endl;
    ntb::RPCClient rpc_client(rpc_endpoint);

    // Decoding seed
    std::string ed25519_seed_b58 = argv[3];
    std::array<uint8_t, 32> ed25519_seed;
    if (!base58::decode(ed25519_seed_b58.data(), ed25519_seed.data())) {
        std::cerr << "Invalid base58 NEAR seed" << std::endl;
        return 1;
    }

    // Building Keypair from seed
    std::array<uint8_t, 32> public_key = {};
    std::array<uint8_t, 64> private_key = {};
    ed25519_create_keypair(public_key.data(), private_key.data(), ed25519_seed.data());
    std::string public_key_b58 = base58::encode(public_key.data(), public_key.data() + public_key.size());
    std::cout << "Public Key : " << public_key_b58 << std::endl;

    // Retrieving AccessKey details
    nlohmann::json access_key_query_parameters = {
        { "request_type", "view_access_key" },
        { "finality", "final" },
        { "account_id", sender },
        { "public_key", fmt::format("ed25519:{}", public_key_b58) }
    };
    auto access_key_resp = rpc_client.query(access_key_query_parameters);
    auto access_key_result = access_key_resp.expect("failed to get AccessKey details").data;

    // Asserting AccessKey has sufficient permissions
    if (access_key_result["permission"] != "FullAccess") {
        std::cerr << "Account " << sender << " does not have permission to send tokens using key " << public_key_b58 << std::endl;
        return 1;
    }

    // Using next nonce value
    uint64_t nonce = access_key_result["nonce"];
    nonce++;
    std::cout << "Using nonce : " << nonce << std::endl;

    // Single FunctionCall action
    nlohmann::json sc_call_parameters = { { "message", "hello :)" } };
    std::string sc_call_parameters_b64 = base64::encode(sc_call_parameters.dump());
    std::vector<uint8_t> sc_call_parameters_bytes(sc_call_parameters_b64.begin(), sc_call_parameters_b64.end());
    std::vector<ntb::schemas::Action> actions = { ntb::schemas::FunctionCall { "set_greeting", sc_call_parameters_bytes, MAX_GAS, 0 } };

    // Retrieving recent block hash
    std::string block_hash_b58 = access_key_result["block_hash"];
    std::array<uint8_t, 32> recent_block_hash;
    if (!base58::decode(block_hash_b58, recent_block_hash.data())) {
        std::cerr << fmt::format("Could not decode recent block hash '{}'", block_hash_b58) << std::endl;
    }
    std::cout << "Block hash : " << block_hash_b58 << std::endl;

    // Building Transaction struct
    ntb::schemas::Transaction transaction {
        sender,
        ntb::schemas::ED25519PublicKey { public_key },
        nonce,
        receiver,
        recent_block_hash,
        actions,
    };

    // Encoding Transaction using Borsh
    BorshEncoder tx_encoder;
    tx_encoder.Encode(transaction);
    std::cout << std::endl;
    std::cout << "Final bytes : ";
    for (auto c : tx_encoder.GetBuffer()) {
        printf("%d, ", c);
    }
    printf("\n");
    std::vector<uint8_t> tx_bytes = tx_encoder.GetBuffer();

    // Building Transaction signature
    sha256::SHA256 tx_hasher;
    tx_hasher.update(tx_bytes.data(), tx_bytes.size());
    sha256::hash_container tx_hash = tx_hasher.digest();
    std::array<uint8_t, 64> signature = {};
    std::cout << "Message to sign : " << base58::encode(tx_hash.data(), tx_hash.data() + tx_hash.size()) << std::endl;
    ed25519_sign(signature.data(), tx_hash.data(), tx_hash.size(), public_key.data(), private_key.data());

    // Building SignedTransaction
    ntb::schemas::SignedTransaction signed_transaction { transaction, ntb::schemas::Signature { ntb::schemas::KeyType::ED25519, signature } };
    BorshEncoder signed_tx_encoder;
    signed_tx_encoder.Encode(signed_transaction);
    std::vector<uint8_t> signed_tx_bytes = signed_tx_encoder.GetBuffer();

    // Broadcasting transaction
    nlohmann::json broadcast_tx_parameters = { base64::encode(signed_tx_bytes.data(), signed_tx_bytes.size()) };
    auto broadcast_resp = rpc_client.call("broadcast_tx_commit", broadcast_tx_parameters);
    auto broadcast_result = broadcast_resp.expect("transaction failed to broadcast").data;
    std::cout << "Transaction broadcasted at " << broadcast_result["transaction"]["hash"] << std::endl;

    // Smart contract call (view)
    nlohmann::json rpc_sc_call_view_parameters = {
        { "request_type", "call_function" },
        { "finality", "final" },
        { "account_id", "akc.nearcon22.testnet" },
        { "method_name", "get_greeting" },
        { "args_base64", "e30=" }
    };
    auto sc_call_view_resp = rpc_client.query(rpc_sc_call_view_parameters);
    auto sc_call_view_result = sc_call_view_resp.expect("failed to query smart contract").data;
    std::cout << sc_call_view_result.dump() << std::endl;

    return 0;
}

int main(int argc, char** argv)
{
    // return tx_example(argc, argv);
    return tx_example_simple(argc, argv);
    // return smart_contract_call_example(argc, argv);
}