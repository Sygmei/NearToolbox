#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include <ntb/near.hpp>

// Based on https://github.com/near-examples/transaction-examples/blob/master/send-tokens-deconstructed.js

int tx_example_simple([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    const std::string sender = argv[1];
    const std::string receiver = argv[2];
    const std::string network_id = "testnet";

    ntb::NearClient near_client(network_id, ntb::ED25519Keypair(argv[3]), ntb::NamedAccount { sender });
    near_client.transfer(receiver, "0.84");

    return 0;
}

int smart_contract_call_example([[maybe_unused]] int argc, char** argv)
{
    const std::string sender = argv[1];
    const std::string contract_address = argv[2];
    const std::string network_id = "testnet";

    ntb::NearClient near_client(network_id, ntb::ED25519Keypair(argv[3]), ntb::NamedAccount{ sender });
    near_client.contract_call(contract_address, "set_greeting", { { "message", "hello :)" } });
    auto get_greeting_result = near_client.contract_view(contract_address, "get_greeting");

    std::cout << "Result " << get_greeting_result.result.dump() << std::endl;

    return 0;
}

int main(int argc, char** argv)
{
    // return tx_example_simple(argc, argv);
    return smart_contract_call_example(argc, argv);
}