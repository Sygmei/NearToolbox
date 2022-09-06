#include <string>

#include <ntb/rpc.hpp>

namespace ntb
{
    RPCClient::RPCClient(const std::string &endpoint)
        : m_endpoint(endpoint), m_client(endpoint)
    {
    }

    httplib::Result RPCClient::call_raw(const nlohmann::json &payload)
    {
        auto resp = m_client.Post("/", payload.dump(), "application/json");
        return resp;
    }

    RPCResponse RPCClient::call(const std::string &method_name, const nlohmann::json &parameters)
    {
        nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"id", "dontcare"},
            {"method", method_name},
            {"params", parameters}};
        auto resp = call_raw(payload);

        if (resp->status >= 200 && resp->status <= 299)
        {
            auto result_or_error = nlohmann::json::parse(resp->body);
            if (result_or_error.contains("result"))
            {
                return RPCSuccess{result_or_error["result"]};
            }
            else
            {
                return cpp::fail(RPCError{result_or_error["error"]});
            }
        }
        else
        {
            return cpp::fail(RPCError{});
        }
    }

    RPCResponse RPCClient::query(const nlohmann::json &parameters)
    {
        return call("query", parameters);
    }
}
