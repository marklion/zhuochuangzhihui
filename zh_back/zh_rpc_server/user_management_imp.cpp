#include "user_management_imp.h"
#include "../zh_database/zh_db_config.h"
#include "zh_rpc_util.h"

user_management_handler *user_management_handler::m_inst = nullptr;

void user_management_handler::get_all_user(std::vector<user_info> &_return)
{
}
void user_management_handler::self_info(user_info &_return, const std::string &ssid)
{
    auto user_info = zh_rpc_util_get_online_user(ssid);
    if (!user_info)
    {
        ZH_RETURN_UNLOGIN_MSG();
    }

    auto permission = user_info->get_parent<zh_sql_user_permission>("permission");
    if (permission)
    {
        _return.permission = permission->key;
    }
    _return.id = user_info->get_pri_id();
    _return.name = user_info->name;
    _return.phone = user_info->phone;
}
void user_management_handler::user_login(std::string &_return, const std::string &phone, const std::string &password)
{
    auto user_info = sqlite_orm::search_record<zh_sql_user_info>("phone == '%s' AND password == '%s'", phone.c_str(), password.c_str());
    if (!user_info)
    {
        ZH_RETURN_MSG("用户名或密码错误");
    }
    auto user_login = user_info->get_children<zh_sql_user_login>("online_user");
    if (user_login)
    {
        user_login->remove_record();
    }
    zh_sql_user_login new_login;
    new_login.ssid = zh_rpc_util_gen_ssid();
    new_login.timestamp = time(NULL) / 3600;
    new_login.set_parent(*user_info, "online_user");
    if (new_login.insert_record())
    {
        _return = new_login.ssid;
    }
}