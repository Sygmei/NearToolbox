#include <iostream>
#include <string>

#include <ntb/near.hpp>

// Based on https://github.com/near-examples/transaction-examples/blob/master/send-tokens-deconstructed.js

int main([[maybe_unused]] int argc, char **argv)
{
    const std::string sender = argv[1];
    const std::string receiver = argv[2];
    const std::string network_id = "testnet";

    ntb::NearClient near_client(network_id, ntb::LedgerWallet{}, ntb::NamedAccount{sender});
    ntb::TransactionResult tx_result = near_client.transfer(receiver, "0.84");

    std::cout << "Transaction : " << tx_result.tx_data.dump() << std::endl;

    return 0;
}