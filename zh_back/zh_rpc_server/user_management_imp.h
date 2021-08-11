#if !defined(_USER_MANAGEMENT_H_)
#define _USER_MANAGEMENT_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/user_management.h"

class user_management_handler : virtual public user_managementIf
{
    static user_management_handler *m_inst;
public:
    static user_management_handler* get_inst() {
        if (m_inst == nullptr)
        {
            m_inst = new user_management_handler();
        }
        return m_inst;
    }
    virtual void get_all_user(std::vector<user_info> &_return);
    virtual void self_info(user_info &_return, const std::string &ssid);
    virtual void user_login(std::string &_return, const std::string &phone, const std::string &password);
};

#endif // _USER_MANAGEMENT_H_
