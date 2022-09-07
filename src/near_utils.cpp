#include <algorithm>
#include <vector>

#include <fmt/format.h>

#include <ntb/near_utils.hpp>
#include <ntb/string_utils.hpp>

namespace ntb
{
    std::optional<std::string> parse_near_amount(std::string amount)
    {
        if (amount.empty())
        {
            return std::nullopt;
        }
        amount = cleanup_amount(amount);
        const std::vector<std::string> split_amount = split(amount, ".");
        const std::string whole_part = split_amount[0];
        const std::string frac_part = (split_amount.size() > 1) ? split_amount[1] : "";
        if (split_amount.size() > 2 || frac_part.size() > NEAR_NOMINATION_EXP)
        {
            throw std::runtime_error(fmt::format("Cannot parse '{}' as NEAR amount", amount));
        }
        return ltrim(whole_part + rpad(frac_part, NEAR_NOMINATION_EXP, '0'));
    }

    std::string cleanup_amount(std::string amount)
    {
        amount = replace(amount, ",", "");
        return trim(amount);
    }

    std::vector<uint8_t> bip32_path_to_bytes(std::string bip32_path, bool prepend_length) {
        constexpr uint32_t HARDENED_FLAG = 0x80000000;

        std::transform(bip32_path.begin(), bip32_path.end(), bip32_path.begin(),
            [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
        if (bip32_path.substr(0, 2) == "m/")
        {
            bip32_path = bip32_path.substr(2);
        }

        const std::vector<std::string> path_parts = split(bip32_path, "/");
        std::vector<uint8_t> bytes;
        bytes.reserve(path_parts.size() * 4 + ((prepend_length) ? 1 : 0));
        if (prepend_length)
        {
            bytes.push_back(static_cast<uint8_t>(path_parts.size()));
        }
        for (std::string part : path_parts)
        {
            bool hardened = false;
            if (ends_with(part, "'"))
            {
                hardened = true;
                part = part.substr(0, part.size() - 1);
            }
            if (!is_string_int(part))
            {
                throw std::runtime_error("invalid bip32 path");
            }
            uint32_t part_as_uint32 = std::stoul(part);
            if (hardened)
            {
                part_as_uint32 += HARDENED_FLAG;
            }
            bytes.push_back(static_cast<uint8_t>(part_as_uint32 >> 24));
            bytes.push_back(static_cast<uint8_t>(part_as_uint32 >> 16));
            bytes.push_back(static_cast<uint8_t>(part_as_uint32 >> 8));
            bytes.push_back(static_cast<uint8_t>(part_as_uint32));
        }
        return bytes;
    }
}
