#include "user_management_imp.h"
#include "../zh_database/zh_db_config.h"
#include "zh_rpc_util.h"

user_management_handler *user_management_handler::m_inst = nullptr;


bool user_management_handler::change_user_password(const std::string &ssid, const std::string &password)
{
    bool ret = false;
    auto user_info = zh_rpc_util_get_online_user(ssid);
    if (!user_info)
    {
        ZH_RETURN_UNLOGIN_MSG();
    }
    if (user_info->password == password)
    {
        ZH_RETURN_MSG("不允许使用相同密码");
    }
    user_info->password = password;
    user_info->need_change_password = 0;
    ret = user_info->update_record();

    return ret;
}
bool user_management_handler::del_user(const std::string &ssid, const int64_t user_id)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 0);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto user_need_del = sqlite_orm::search_record<zh_sql_user_info>(user_id);
    if (!user_need_del)
    {
        ZH_RETURN_MSG("用户不存在");
    }
    if (user_need_del->name == "admin")
    {
        ZH_RETURN_MSG("无法删除内置管理员");
    }
    user_need_del->remove_record();

    return true;
}
bool user_management_handler::reset_user(const std::string &ssid, const int64_t user_id, const std::string &password)
{
    bool ret = false;

    auto opt_user = zh_rpc_util_get_online_user(ssid, 0);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto user_need_reset = sqlite_orm::search_record<zh_sql_user_info>(user_id);
    if (!user_need_reset)
    {
        ZH_RETURN_MSG("用户不存在");
    }
    if (user_need_reset->name == "admin")
    {
        ZH_RETURN_MSG("无法重置内置管理员");
    }
    user_need_reset->password = password;
    user_need_reset->need_change_password = 1;
    ret = user_need_reset->update_record();

    return ret;
}
bool user_management_handler::add_user(const std::string &ssid, const user_info &new_user)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 0);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto exist_user = sqlite_orm::search_record<zh_sql_user_info>("phone == '%s'", new_user.phone.c_str());
    if (exist_user)
    {
        ZH_RETURN_MSG("该手机号已注册过");
    }
    auto required_permission = sqlite_orm::search_record<zh_sql_user_permission>("name == '%s'", new_user.permission_name.c_str());
    if (!required_permission)
    {
        ZH_RETURN_MSG("无法分配权限");
    }

    zh_sql_user_info tmp;
    tmp.name = new_user.name;
    tmp.password = new_user.password;
    tmp.phone = new_user.phone;
    tmp.set_parent(*required_permission, "permission");
    tmp.need_change_password = 1;

    return tmp.insert_record();
}
void user_management_handler::get_all_permissions(std::vector<user_permission> &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 2);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto all_permissions = sqlite_orm::search_record_all<zh_sql_user_permission>();
    for (auto &itr : all_permissions)
    {
        user_permission tmp;
        tmp.description = itr.description;
        tmp.id = itr.get_pri_id();
        tmp.name = itr.name;
        _return.push_back(tmp);
    }
}
void user_management_handler::get_all_user(std::vector<user_info> &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 0);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    auto all_user = sqlite_orm::search_record_all<zh_sql_user_info>();
    for (auto &itr : all_user)
    {
        user_info tmp;
        auto permission = itr.get_parent<zh_sql_user_permission>("permission");
        if (permission)
        {
            tmp.permission_name = permission->name;
            tmp.permission = permission->key;
        }
        tmp.id = itr.get_pri_id();
        tmp.name = itr.name;
        tmp.phone = itr.phone;
        _return.push_back(tmp);
    }
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
        _return.permission_name = permission->name;
    }
    _return.id = user_info->get_pri_id();
    _return.name = user_info->name;
    _return.phone = user_info->phone;
    _return.need_change_password = user_info->need_change_password == 0 ? false : true;
}
void user_management_handler::user_login(std::string &_return, const std::string &phone, const std::string &password)
{
    auto user_info = sqlite_orm::search_record<zh_sql_user_info>("phone == '%s' AND password == '%s'", phone.c_str(), password.c_str());
    if (!user_info)
    {
        ZH_RETURN_MSG("用户名或密码错误");
    }
    auto user_login = user_info->get_children<zh_sql_user_login>("online_user");
    if (user_login && !zh_rpc_util_get_online_user(user_login->ssid))
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

void user_management_handler::user_logoff(const std::string &ssid)
{
    auto user_info = zh_rpc_util_get_online_user(ssid);
    if (!user_info)
    {
        ZH_RETURN_UNLOGIN_MSG();
    }
    auto user_login = user_info->get_children<zh_sql_user_login>("online_user");
    if (user_login)
    {
        user_login->remove_record();
    }
}