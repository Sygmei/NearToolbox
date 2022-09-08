#include <iostream>
#include <string>

#include <ntb/near.hpp>

int smart_contract_call_example([[maybe_unused]] int argc, char **argv)
{
    const std::string sender = argv[1];
    const std::string contract_address = argv[2];
    const std::string network_id = "testnet";

    ntb::NearClient near_client(network_id, ntb::ED25519Keypair(argv[3]), ntb::NamedAccount{sender});
    near_client.contract_call(contract_address, "set_greeting", {{"message", "hello :)"}});
    auto get_greeting_result = near_client.contract_view(contract_address, "get_greeting");

    std::cout << "Result " << get_greeting_result.result.dump() << std::endl;

    return 0;
}