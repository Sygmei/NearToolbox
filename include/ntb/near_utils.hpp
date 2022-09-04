#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace ntb
{
    constexpr uint32_t NEAR_NOMINATION_EXP = 24;

    std::optional<std::string> parse_near_amount(std::string amount);
    std::string cleanup_amount(std::string amount);
}