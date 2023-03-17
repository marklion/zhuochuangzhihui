#if !defined(_USER_MANAGEMENT_H_)
#define _USER_MANAGEMENT_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/user_management.h"
#include "../zh_database/zh_db_config.h"

class user_management_handler : virtual public user_managementIf
{
    static user_management_handler *m_inst;

public:
    static user_management_handler *get_inst()
    {
        if (m_inst == nullptr)
        {
            m_inst = new user_management_handler();
        }
        return m_inst;
    }
    virtual void get_all_permissions(std::vector<user_permission> &_return, const std::string &ssid);
    virtual void get_all_user(std::vector<user_info> &_return, const std::string &ssid);
    virtual void self_info(user_info &_return, const std::string &ssid);
    virtual void user_login(std::string &_return, const std::string &phone, const std::string &password);
    virtual void user_logoff(const std::string &ssid);
    virtual bool add_user(const std::string &ssid, const user_info &new_user);
    virtual bool del_user(const std::string &ssid, const int64_t user_id);
    virtual bool reset_user(const std::string &ssid, const int64_t user_id, const std::string &password);
    virtual bool change_user_password(const std::string &ssid, const std::string &password);
    virtual void get_user_permission_target(std::vector<permission_target_info> &_return, const int64_t user_id);
    virtual bool add_user_permission_target(const std::string &ssid, const int64_t user_id, const permission_target_info &target_info);
    virtual void del_user_permission_target(const std::string &ssid, const int64_t user_id, const int64_t target_info_id);
    virtual void get_all_permission_item(std::vector<permission_target_info> &_return);
    bool pri_add_user_permission_target(const int64_t user_id, const permission_target_info &target_info);
    std::string pri_user_login(zh_sql_user_info &user_info);
};

#endif // _USER_MANAGEMENT_H_
