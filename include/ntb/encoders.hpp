#pragma once

#include <vector>

#include <bignumber/bignumber.hpp>

namespace ntb::encoders
{
    std::vector<uint8_t> encode_big_number(const BigNumber& number);
}

ENCODABLE_STRUCT(BigNumber, ntb::encoders::encode_big_number);