#pragma once

#include <string>
#include <string_view>

namespace base64 {
constexpr std::string_view base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::string encode(const std::string& data);
std::string encode(const unsigned char* data, size_t len);
std::string decode(const std::string& data);
std::string decode(const unsigned char* data, size_t len);
}