#if !defined(_SYSTEM_MANAGEMENT_IMP_H_)
#define _SYSTEM_MANAGEMENT_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/system_management.h"

class system_management_handler : virtual public system_managementIf {
private:
    static system_management_handler *m_inst;
    system_management_handler() {}
public:
    static system_management_handler *get_inst() {
        if (m_inst == nullptr)
        {
            m_inst = new system_management_handler();
        }
        return m_inst;
    }
    virtual bool reboot_system(const std::string &ssid);
    virtual void current_version(std::string &_return);
    virtual void get_device_config(device_config &_return, const std::string &ssid);
    virtual bool edit_device_config(const std::string &ssid, const device_config &config);
};

#endif // _SYSTEM_MANAGEMENT_IMP_H_
