#include "near_client_wrapper.h"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <ntb/signer.hpp>

using namespace godot;

std::string godot_string_to_std_string(String str)
{
    const char32_t *ws_str = str.ptr();
    return std::string(ws_str, ws_str + str.length());
}

void NearClientWrapper::_notification(int p_what)
{
    // UtilityFunctions::print("Notification: ", String::num(p_what));
}

bool NearClientWrapper::_set(const StringName &p_name, const Variant &p_value)
{
    if (p_name == StringName("private_key"))
    {
        m_private_key = p_value;
        return true;
    }
    else if (p_name == StringName("network"))
    {
        m_network = p_value;
        return true;
    }
    else if (p_name == StringName("account_id"))
    {
        m_account_id = p_value;
        return true;
    }
    return false;
}

bool NearClientWrapper::_get(const StringName &p_name, Variant &r_ret) const
{
    if (p_name == StringName("private_key"))
    {
        r_ret = m_private_key;
        return true;
    }
    else if (p_name == StringName("network"))
    {
        r_ret = m_network;
        return true;
    }
    else if (p_name == StringName("account_id"))
    {
        r_ret = m_account_id;
        return true;
    }
    return false;
}

String NearClientWrapper::_to_string() const
{
    return "[ GDExtension::NearClient <--> Instance ID:" + itos(get_instance_id()) + " ]";
}

void NearClientWrapper::_get_property_list(List<PropertyInfo> *p_list) const
{
    p_list->push_back(PropertyInfo(Variant::STRING, "private_key"));
    p_list->push_back(PropertyInfo(Variant::STRING, "network"));
    p_list->push_back(PropertyInfo(Variant::STRING, "account_id"));
}

bool NearClientWrapper::_property_can_revert(const StringName &p_name) const
{
    if (p_name == StringName("network"))
    {
        return true;
    }
    return false;
};

bool NearClientWrapper::_property_get_revert(const StringName &p_name, Variant &r_property) const
{
    if (p_name == StringName("network"))
    {
        r_property = "mainnet";
        return true;
    }
    return false;
};

void NearClientWrapper::_bind_methods()
{
    // Methods.
    ClassDB::bind_method(D_METHOD("login"), &NearClientWrapper::login);
    ClassDB::bind_method(D_METHOD("transfer"), &NearClientWrapper::transfer);
    ClassDB::bind_method(D_METHOD("contract_call"), &NearClientWrapper::contract_call);
    ClassDB::bind_method(D_METHOD("contract_view"), &NearClientWrapper::contract_view);
}

NearClientWrapper::NearClientWrapper()
{
    UtilityFunctions::print("Building NearClient");
}

NearClientWrapper::~NearClientWrapper()
{
    UtilityFunctions::print("Destructor.");
}

void NearClientWrapper::login()
{
    std::string network_str = godot_string_to_std_string(m_network);
    std::string b58_seed_or_private_key_str = godot_string_to_std_string(m_private_key);
    std::string account_id_str = godot_string_to_std_string(m_account_id);
    auto signing_method = ntb::ED25519Keypair(b58_seed_or_private_key_str);
    m_near = std::make_unique<ntb::NearClient>(network_str, std::move(signing_method), ntb::NamedAccount{account_id_str});
}

void NearClientWrapper::transfer(String recipient, String amount)
{
    std::string recipient_str = godot_string_to_std_string(recipient);
    std::string amount_str = godot_string_to_std_string(amount);
    m_near->transfer(recipient_str, amount_str);
}

void NearClientWrapper::contract_call(String contract_address, String method_name, String payload, String deposit)
{
    std::string contract_address_str = godot_string_to_std_string(contract_address);
    std::string method_name_str = godot_string_to_std_string(method_name);
    std::string payload_str = godot_string_to_std_string(payload);
    std::string deposit_str = godot_string_to_std_string(deposit);
    nlohmann::json payload_object = nlohmann::json::parse(payload_str);
    m_near->contract_call(contract_address_str, method_name_str, payload_object, deposit_str);
}

void NearClientWrapper::contract_view(String contract_address, String method_name, String payload)
{
    std::string contract_address_str = godot_string_to_std_string(contract_address);
    std::string method_name_str = godot_string_to_std_string(method_name);
    std::string payload_str = godot_string_to_std_string(payload);
    nlohmann::json payload_object = nlohmann::json::parse(payload_str);
    m_near->contract_view(contract_address_str, method_name_str, payload_object);
}