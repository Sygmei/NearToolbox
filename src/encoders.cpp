#include <vector>

#include <bignumber/bignumber.hpp>
#include <borsh/visit_struct.hpp>

namespace ntb::encoders
{
    std::vector<uint8_t> encode_big_number(const BigNumber &number)
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
        std::vector<uint8_t> bytes(16, 0); // TODO: Use template type that indicates dump size in schemas
        for (int32_t byte_index = bytes_required - 1; byte_index >= 0; byte_index--)
        {
            BigNumber divisor = BigNumber(256).pow(byte_index);
            if (const BigNumber byte_value = number_copy / divisor; byte_value > 0)
            {
                // Little-endian encoding
                bytes[byte_index] = static_cast<uint8_t>(std::stoi(byte_value.as_string()));
                number_copy = number_copy - (divisor * byte_value);
            }
        }
        return bytes;
    }
}