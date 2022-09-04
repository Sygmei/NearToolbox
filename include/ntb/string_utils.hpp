#pragma once

#include <string>
#include <vector>

namespace ntb
{
    std::string ltrim(std::string s);
    std::string ltrim(std::string s, unsigned char trim_char);
    std::string rtrim(std::string s);
    std::string rtrim(std::string s, unsigned char trim_char);
    std::string trim(std::string s);
    std::string trim(std::string s, unsigned char trim_char);
    std::string replace(std::string str, const std::string& from, const std::string& to);
    std::vector<std::string> split(const std::string& str, const std::string& delimiters);
    std::string lpad(std::string str, const size_t num, const char padding_char = ' ');
    std::string rpad(std::string str, const size_t num, const char padding_char = ' ');
}