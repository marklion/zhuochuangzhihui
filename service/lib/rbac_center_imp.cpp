#include "rpc_include.h"
#include "rbac_center_imp.h"
rbac_center_handler::rbac_center_handler()
{
    struct perm_conf
    {
        bool is_module = false;
        std::string permission_name;
        std::string text_name;
    } init_conf[] = {
        {true, "config", "配置管理"},
        {false, "stuff", "物料"},
    };

    for (auto &itr : init_conf)
    {
        auto es = sqlite_orm::search_record<sql_permission>("permission_name == '%s'", itr.permission_name.c_str());
        if (!es)
        {
            sql_permission tmp;
            tmp.is_module = itr.is_module;
            tmp.permission_name = itr.permission_name;
            tmp.text_name = itr.text_name;

            tmp.insert_record();
        }
    }

    auto es = sqlite_orm::search_record<sql_role>("name == 'admin'");
    if (!es)
    {
        sql_role tmp;
        tmp.name = "admin";
        tmp.read_only = 0;
        auto all_perm = sqlite_orm::search_record_all<sql_permission>();
        std::vector< std::string> perm_ids;
        for (auto &itr:all_perm)
        {
            perm_ids.push_back(std::to_string( itr.get_pri_id()));
        }
        tmp.has_permission_id = util_join_string(perm_ids);
        tmp.insert_record();
    }
}
bool rbac_center_handler::has_permission(const std::string &token, const std::string &target_module, const std::string &target_resouce, const bool is_write)
{
    bool ret = false;

    auto user = db_get_online_user(token);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto tm = sqlite_orm::search_record<sql_permission>("permission_name == '%s' AND is_module!= 0", target_module.c_str());
    auto tr = sqlite_orm::search_record<sql_permission>("permission_name == '%s' AND is_module == 0", target_resouce.c_str());
    if (tr && tm)
    {
        std::string write_require = "PRI_ID != 0";
        if (is_write)
        {
            write_require = "read_only == 0";
        }
        auto roles = sqlite_orm::search_record_all<sql_role>(write_require);
        for (auto &itr : roles)
        {
            auto contain_users = util_split_string(itr.has_user_id);
            auto contain_permissions = util_split_string(itr.has_permission_id);
            if (util_vector_has(contain_users, std::to_string(user->get_pri_id())) &&
                util_vector_has(contain_permissions, std::to_string(tm->get_pri_id())) &&
                util_vector_has(contain_permissions, std::to_string(tr->get_pri_id())))
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}
bool rbac_center_handler::add_role(const rbac_role &new_one)
{
    bool ret = false;

    return ret;
}
bool rbac_center_handler::del_role(const int64_t role_id)
{
    bool ret = false;

    return ret;
}
void rbac_center_handler::get_all_roles(std::vector<rbac_role> &_return)
{
    auto roles = sqlite_orm::search_record_all<sql_role>();
    for (auto &itr:roles)
    {
        rbac_role tmp;
        auto user_ids = util_split_string(itr.has_user_id);
        auto perm_ids = util_split_string(itr.has_permission_id);
        for (auto &uid:user_ids)
        {
            auto user = sqlite_orm::search_record<sql_user>(atoi(uid.c_str()));
            if (user)
            {
                rbac_user t_user;
                t_user.id = user->get_pri_id();
                t_user.md5_password = user->md5_password;
                t_user.name = user->name;
                t_user.phone = user->phone;
                tmp.all_user.push_back(t_user);
            }
        }
        for (auto &pid:perm_ids)
        {
            auto perm = sqlite_orm::search_record<sql_permission>(atoi(pid.c_str()));
            if (perm)
            {
                if (perm->is_module)
                {
                    tmp.author_modules.push_back(perm->text_name);
                }
                else
                {
                    tmp.author_resouces.push_back(perm->text_name);
                }
            }
        }
        tmp.id = itr.get_pri_id();
        tmp.read_only = itr.read_only;
        tmp.role_name = itr.name;

        _return.push_back(tmp);
    }
}
bool rbac_center_handler::add_user_to_role(const int64_t role_id, const int64_t user_id)
{
    bool ret = false;

    auto role = sqlite_orm::search_record<sql_role>(role_id);
    if (!role)
    {
        ZH_RETURN_MSG("角色不存在");
    }
    if (!sqlite_orm::search_record<sql_user>(user_id))
    {
        ZH_RETURN_MSG("用户不存在");
    }

    auto cur_users = util_split_string( role->has_user_id);
    if (!util_vector_has(cur_users, std::to_string(user_id)))
    {
        cur_users.push_back(std::to_string(user_id));
    }

    role->has_user_id = util_join_string(cur_users);
    ret = role->update_record();

    return ret;
}
bool rbac_center_handler::del_user_from_role(const int64_t role_id, const int64_t user_id)
{
    bool ret = false;
    auto role = sqlite_orm::search_record<sql_role>(role_id);
    if (!role)
    {
        ZH_RETURN_MSG("角色不存在");
    }
    if (!sqlite_orm::search_record<sql_user>(user_id))
    {
        ZH_RETURN_MSG("用户不存在");
    }
    auto cur_users = util_split_string(role->has_user_id);
    std::vector<std::string> new_users;
    for (auto &itr : cur_users)
    {
        if (itr != std::to_string(user_id))
        {
            new_users.push_back(itr);
        }
    }

    role->has_user_id = util_join_string(new_users);
    ret = role->update_record();

    return ret;
}
bool rbac_center_handler::add_role_module_permission(const int64_t role_id, const std::string &target_module)
{
    bool ret = false;

    return ret;
}
bool rbac_center_handler::del_role_module_permission(const int64_t role_id, const std::string &target_module)
{
    bool ret = false;

    return ret;
}
bool rbac_center_handler::add_role_resource_permission(const int64_t role_id, const std::string &target_resouce)
{
    bool ret = false;

    return ret;
}
bool rbac_center_handler::del_role_resource_permission(const int64_t role_id, const std::string &target_resouce)
{
    bool ret = false;

    return ret;
}
bool rbac_center_handler::add_user(const rbac_user &new_one)
{
    bool ret = false;
    sql_user tmp;
    tmp.name = new_one.name;
    tmp.md5_password = new_one.md5_password;
    tmp.phone = new_one.phone;
    tmp.is_external = 0;
    ret = tmp.insert_record();
    return ret;
}

void rbac_center_handler::del_user(const int64_t user_id)
{
    auto exist_record = sqlite_orm::search_record<sql_user>(user_id);
    if (exist_record)
    {
        exist_record->remove_record();
    }
}

void rbac_center_handler::get_all_user(std::vector<rbac_user> &_return)
{
    auto users = sqlite_orm::search_record_all<sql_user>();
    for (auto &itr : users)
    {
        rbac_user tmp;
        tmp.id = itr.get_pri_id();
        tmp.md5_password = itr.md5_password;
        tmp.name = itr.name;
        tmp.phone = itr.phone;
        auto roles = sqlite_orm::search_record_all<sql_role>();
        for (auto &single_role : roles)
        {
            auto contain_users = util_split_string(single_role.has_user_id);
            if (util_vector_has(contain_users, std::to_string(itr.get_pri_id())))
            {
                tmp.role_name.push_back(single_role.name);
            }
        }
        _return.push_back(tmp);
    }
}

void rbac_center_handler::get_all_permission(std::vector<std::string> &_return)
{
    auto aps = sqlite_orm::search_record_all<sql_permission>();
    for (auto &itr : aps)
    {
        std::string tmp;
        if (itr.is_module)
        {
            tmp += "模块:";
        }
        else
        {
            tmp += "资源:";
        }
        tmp += itr.text_name;
        _return.push_back(tmp);
    }
}
