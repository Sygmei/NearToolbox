#ifndef NEAR_CLIENT_WRAPPER_CLASS_H
#define NEAR_CLIENT_WRAPPER_CLASS_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include <ntb/near.hpp>

using namespace godot;

class NearClientWrapper : public Node
{
    GDCLASS(NearClientWrapper, Node)

protected:
    static void _bind_methods();

    void _notification(int p_what);
    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;
    bool _property_can_revert(const StringName &p_name) const;
    bool _property_get_revert(const StringName &p_name, Variant &r_property) const;

    String _to_string() const;

private:
    std::unique_ptr<ntb::NearClient> m_near;
    String m_network;
    String m_private_key;
    String m_account_id;

public:
    NearClientWrapper();
    ~NearClientWrapper();

    void login();
    void transfer(String recipient, String amount);
    void contract_call(String contract_address, String method_name, String payload, String deposit);
    void contract_view(String contract_address, String method_name, String payload);
};

#endif // NEAR_CLIENT_WRAPPER_CLASS_H
