#include <string>

#include <ntb/rpc.hpp>

namespace ntb
{
    RPCClient::RPCClient(const std::string& endpoint)
        : m_endpoint(endpoint), m_client(endpoint)
    {
    }

    nlohmann::json RPCClient::call(const std::string& method_name, const nlohmann::json& parameters)
    {
        nlohmann::json payload = {
            { "jsonrpc", "2.0" },
            { "id", "dontcare" },
            { "method", method_name },
            { "params", parameters }
        };
        auto resp = m_client.Post("/", payload.dump(), "application/json");
        return nlohmann::json::parse(resp->body)["result"];
    }

    nlohmann::json RPCClient::query(const nlohmann::json& parameters)
    {
        return call("query", parameters);
    }
}
