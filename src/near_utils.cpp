#include <vector>

#include <fmt/format.h>

#include <near_utils.hpp>
#include <string_utils.hpp>

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
}