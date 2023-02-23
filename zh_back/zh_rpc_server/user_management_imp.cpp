#include "user_management_imp.h"
#include "../zh_database/zh_db_config.h"
#include "zh_rpc_util.h"

user_management_handler *user_management_handler::m_inst = nullptr;

std::map<std::string, std::string> g_permisson_description_map = {
    {ZH_PERMISSON_TARGET_CASH, "财务操作"},
    {ZH_PERMISSON_TARGET_USER, "系统配置"},
    {ZH_PERMISSON_TARGET_ORDER, "订单管理"},
    {ZH_PERMISSON_TARGET_FIELD, "现场操作"}

};
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
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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

    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
    auto opt_user = zh_rpc_util_get_online_user(ssid);
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
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
        auto bind_targets = itr.get_all_children<zh_sql_permission_target>("belong_user");
        for (auto &single_target:bind_targets)
        {
            permission_target_info tmp_info;
            tmp_info.id = single_target.get_pri_id();
            tmp_info.is_read = single_target.is_read;
            tmp_info.target = single_target.target;
            tmp_info.description = g_permisson_description_map[single_target.target];
            tmp.target_info.push_back(tmp_info);
        }
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

std::string user_management_handler::pri_user_login(zh_sql_user_info &user_info)
{
    std::string ret;

    auto create_new_login_ssid = [&]() -> std::string
    {
        zh_sql_user_login new_login;
        new_login.ssid = zh_rpc_util_gen_ssid();
        new_login.timestamp = time(NULL) / 3600;
        new_login.set_parent(user_info, "online_user");
        new_login.insert_record();
        return new_login.ssid;
    };

    auto user_login = user_info.get_children<zh_sql_user_login>("online_user");
    if (user_login)
    {
        if (!zh_rpc_util_get_online_user(user_login->ssid))
        {
            user_login->remove_record();
            ret = create_new_login_ssid();
        }
        else
        {
            ret = user_login->ssid;
        }
    }
    else
    {
        ret = create_new_login_ssid();
    }

    return ret;
}

void user_management_handler::user_login(std::string &_return, const std::string &phone, const std::string &password)
{
    auto user_info = sqlite_orm::search_record<zh_sql_user_info>("phone == '%s' AND password == '%s'", phone.c_str(), password.c_str());
    if (!user_info)
    {
        ZH_RETURN_MSG("用户名或密码错误");
    }
    _return = pri_user_login(*user_info);
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

void user_management_handler::get_user_permission_target(std::vector<permission_target_info> &_return, const int64_t user_id)
{
    auto user = sqlite_orm::search_record<zh_sql_user_info>(user_id);
    if (user)
    {
        auto pts = user->get_all_children<zh_sql_permission_target>("belong_user");
        for (auto &itr : pts)
        {
            permission_target_info tmp;
            tmp.id = itr.get_pri_id();
            tmp.is_read = itr.is_read;
            tmp.target = itr.target;
            tmp.description = g_permisson_description_map[tmp.target];
            _return.push_back(tmp);
        }
    }
}
bool user_management_handler::pri_add_user_permission_target(const int64_t user_id, const permission_target_info &target_info)
{
    bool ret = false;
    auto opt_user = sqlite_orm::search_record<zh_sql_user_info>(user_id);
    if (opt_user)
    {
        auto exist_record = opt_user->get_children<zh_sql_permission_target>("belong_user", "target == '%s' AND is_read == %ld", target_info.target.c_str(), target_info.is_read);
        if (exist_record)
        {
            ret = true;
        }
        else
        {
            zh_sql_permission_target tmp;
            tmp.target = target_info.target;
            tmp.is_read = target_info.is_read;
            tmp.set_parent(*opt_user, "belong_user");
            ret = tmp.insert_record();
        }
    }

    return ret;
}
bool user_management_handler::add_user_permission_target(const std::string &ssid, const int64_t user_id, const permission_target_info &target_info)
{
    bool ret = false;

    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    auto opt_user = sqlite_orm::search_record<zh_sql_user_info>(user_id);
    if (!user || !opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    ret = pri_add_user_permission_target(user_id, target_info);

    return ret;
}
void user_management_handler::del_user_permission_target(const std::string &ssid, const int64_t user_id, const int64_t target_info_id)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    auto opt_user = sqlite_orm::search_record<zh_sql_user_info>(user_id);
    if (!user || !opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto exist_record = opt_user->get_children<zh_sql_permission_target>("belong_user", "PRI_ID == %ld", target_info_id);
    if (exist_record)
    {
        exist_record->remove_record();
    }
}

void user_management_handler::get_all_permission_item(std::vector<permission_target_info> &_return)
{
    for (auto itr = g_permisson_description_map.begin(); itr != g_permisson_description_map.end(); ++itr)
    {
        permission_target_info tmp;
        tmp.target = itr->first;
        tmp.description = itr->second;
        _return.push_back(tmp);
    }
}