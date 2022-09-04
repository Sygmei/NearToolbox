#pragma once

#include <string>

#include <httplib/httplib.h>
#include <nlohmann/json.hpp>

namespace ntb
{
    class RPCClient
    {
    private:
        std::string m_endpoint;
        httplib::Client m_client;
    public:
        RPCClient(const std::string& endpoint);

        nlohmann::json call(const std::string& method_name, const nlohmann::json& payload);
        nlohmann::json query(const nlohmann::json& parameters);
    };
}