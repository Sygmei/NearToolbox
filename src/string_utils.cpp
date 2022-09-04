#pragma once

#include <algorithm>
#include <cctype>

#include <ntb/string_utils.hpp>

namespace ntb
{
    std::string ltrim(std::string s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
            { return !std::isspace(ch); }));
        return s;
    }

    std::string ltrim(std::string s, unsigned char trim_char)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [trim_char](unsigned char ch)
            { return ch != trim_char; }));
        return s;
    }

    std::string rtrim(std::string s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
            { return !std::isspace(ch); })
            .base(),
            s.end());
        return s;
    }

    std::string rtrim(std::string s, unsigned char trim_char)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [trim_char](unsigned char ch)
            { return ch != trim_char; })
            .base(),
            s.end());
        return s;
    }

    std::string trim(std::string s)
    {
        return ltrim(rtrim(s));
    }

    std::string trim(std::string s, unsigned char trim_char)
    {
        return ltrim(rtrim(s, trim_char), trim_char);
    }

    std::string replace(std::string str, const std::string& from, const std::string& to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return str;
    }

    std::vector<std::string> split(const std::string& str, const std::string& delimiters)
    {
        std::vector<std::string> tokens;
        std::string::size_type last_pos = str.find_first_not_of(delimiters, 0);
        std::string::size_type pos = str.find_first_of(delimiters, last_pos);
        while (std::string::npos != pos || std::string::npos != last_pos)
        {
            tokens.push_back(str.substr(last_pos, pos - last_pos));
            last_pos = str.find_first_not_of(delimiters, pos);
            pos = str.find_first_of(delimiters, last_pos);
        }
        return tokens;
    }

    std::string lpad(std::string str, const size_t num, const char padding_char)
    {
        if (num > str.size())
            str.insert(0, num - str.size(), padding_char);
        return str;
    }

    std::string rpad(std::string str, const size_t num, const char padding_char)
    {
        if (num > str.size())
            str.insert(str.size(), num - str.size(), padding_char);
        return str;
    }

}