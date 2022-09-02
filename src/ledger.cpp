#include <format>
#include <iostream>
#include <string>

#include <base58/base58.hpp>
#include <ed25519/ed25519.h>
#include <httplib/httplib.h>
#include <ledger.h>
#include <nlohmann/json.hpp>
#include <sha256/sha256.hpp>

// Based on https://github.com/near-examples/transaction-examples/blob/master/send-tokens-deconstructed.js

void main()
{
    const std::string sender = "your_address.testnet";
    const std::string receiver = "recipient.testnet";
    const std::string network_id = "testnet";

    sha256::SHA256 hash;
    hash.update("hello");
    auto digested = hash.digest();
    std::cout << sha256::SHA256::to_string(digested) << std::endl;

    std::string rpc_endpoint = std::format("http://rpc.{}.near.org", network_id);

    std::cout << "RPC Endpoint : " << rpc_endpoint << std::endl;

    std::string seed = "your_keypair";
    std::vector<uint8_t> seed_decoded;
    if (!base58::decode(seed.data(), seed_decoded)) {
        std::cerr << "Invalid base58 NEAR seed" << std::endl;
        return;
    }

    std::array<uint8_t, 32> private_key = {};
    std::copy(seed_decoded.begin(), seed_decoded.begin() + 32, private_key.data());
    std::array<uint8_t, 32> public_key = {};
    std::copy(seed_decoded.begin() + 32, seed_decoded.end(), public_key.data());
    std::string public_key_b58 = base58::encode(public_key.data(), public_key.data() + public_key.size());

    httplib::Client http(rpc_endpoint);

    nlohmann::json payload = {
        { "jsonrpc", "2.0" },
        { "id", "dontcare" },
        { "method", "query" },
        { "params",
            { { "request_type", "view_access_key" },
                { "finality", "final" },
                { "account_id", sender },
                { "public_key", std::format("ed25519:{}", public_key_b58) } } }
    };

    auto resp = http.Post("/", payload.dump(), "application/json");
    auto access_key_details = nlohmann::json::parse(resp->body)["result"];

    if (access_key_details["permission"] != "FullAccess") {
        std::cerr << "Account " << sender << " does not have permission to send tokens using key " << public_key_b58 << std::endl;
        return;
    }

    uint32_t nonce = access_key_details["nonce"];
    nonce++;

    return;
}
