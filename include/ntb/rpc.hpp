#pragma once

#include <string>

#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <result/result.hpp>

namespace ntb
{
    struct RPCSuccess
    {
        nlohmann::json data;
    };

    struct RPCError
    {
        nlohmann::json data;
    };

    using RPCResponse = cpp::result<RPCSuccess, RPCError>;

    class RPCClient
    {
    private:
        std::string m_endpoint;
        httplib::Client m_client;

    public:
        RPCClient(const std::string &endpoint);

        httplib::Result call_raw(const nlohmann::json &payload);
        RPCResponse call(const std::string &method_name, const nlohmann::json &payload);
        RPCResponse query(const nlohmann::json &parameters);
    };
}