#include "../lib/rpc_include.h"

#define CLI_MENU_ITEM(x) #x, [](std::ostream &out, std::vector<std::string> _params) { x(out, _params); }

void show_users(std::ostream &out, std::vector<std::string> _params)
{
    std::vector<rbac_user> tmp;
    try
    {
        THR_DEF_CIENT(rbac_center);
        THR_CONNECT(rbac_center);
        client->get_all_user(tmp);
        TRH_CLOSE();
        out << "ID\tname\tphone\tpwd\troles" << std::endl;
        for (auto &itr : tmp)
        {
            std::vector<std::string> user_content;
            user_content.push_back(std::to_string(itr.id));
            user_content.push_back(itr.name);
            user_content.push_back(itr.phone);
            user_content.push_back(itr.md5_password);
            user_content.push_back(util_join_string(itr.role_name));
            out << util_join_string(user_content, "\t") << std::endl;
        }
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void add_user(std::ostream &out, std::vector<std::string> _params)
{
    if (_params.size() != 3)
    {
        out << "参数错误" << std::endl;
    }
    else
    {
        try
        {
            THR_DEF_CIENT(rbac_center);
            THR_CONNECT(rbac_center);
            rbac_user tmp;
            tmp.md5_password = util_calcu_md5(_params[2]);
            tmp.name = _params[0];
            tmp.phone = _params[1];
            client->add_user(tmp);
            TRH_CLOSE();
        }
        catch (gen_exp &e)
        {
            out << e.msg << std::endl;
        }
    }
}

void show_permission(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        THR_DEF_CIENT(rbac_center);
        THR_CONNECT(rbac_center);
        std::vector<rbac_permission> tmp;
        client->get_all_permission(tmp);
        out << "ID\tname\trole_name\ttype" << std::endl;
        for (auto &itr : tmp)
        {
            std::vector<std::string> perm_content;
            perm_content.push_back(std::to_string(itr.id));
            perm_content.push_back(itr.text_name);
            perm_content.push_back(util_join_string(itr.role_name));
            perm_content.push_back(itr.is_module ? "模块" : "资源");
            out << util_join_string(perm_content, "\t") << std::endl;
        }
        TRH_CLOSE();
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void show_roles(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        THR_DEF_CIENT(rbac_center);
        THR_CONNECT(rbac_center);
        std::vector<rbac_role> tmp;
        client->get_all_roles(tmp);
        out << "ID\tname\tusers\tpermissions\ttype" << std::endl;
        for (auto &itr : tmp)
        {
            std::vector<std::string> role_content;
            role_content.push_back(std::to_string(itr.id));
            role_content.push_back(itr.role_name);
            std::vector<std::string> users;
            for (auto &su : itr.all_user)
            {
                users.push_back(su.name);
            }
            std::vector<std::string> perms;
            for (auto &su : itr.author_modules)
            {
                perms.push_back("模块:" + su.text_name);
            }
            for (auto &su : itr.author_resouces)
            {
                perms.push_back("资源:" + su.text_name);
            }
            role_content.push_back(util_join_string(users));
            role_content.push_back(util_join_string(perms));
            role_content.push_back(itr.read_only ? "只读" : "读写");
            out << util_join_string(role_content, "\t") << std::endl;
        }
        TRH_CLOSE();
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void add_user_to_role(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 2)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(rbac_center);
            THR_CONNECT(rbac_center);
            client->add_user_to_role(atoi(_params[0].c_str()), atoi(_params[1].c_str()));
            TRH_CLOSE();
        }
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void del_user_from_role(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 2)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(rbac_center);
            THR_CONNECT(rbac_center);
            client->del_user_from_role(atoi(_params[0].c_str()), atoi(_params[1].c_str()));
            TRH_CLOSE();
        }
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void del_user(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 1)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(rbac_center);
            THR_CONNECT(rbac_center);
            client->del_user(atoi(_params[0].c_str()));
            TRH_CLOSE();
        }
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void add_perm_to_role(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 2)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(rbac_center);
            THR_CONNECT(rbac_center);
            client->add_role_permission(atoi(_params[0].c_str()), atoi(_params[1].c_str()));
            TRH_CLOSE();
        }
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}
void del_perm_from_role(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 2)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(rbac_center);
            THR_CONNECT(rbac_center);
            client->del_role_permission(atoi(_params[0].c_str()), atoi(_params[1].c_str()));
            TRH_CLOSE();
        }
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void add_role(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 2)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(rbac_center);
            THR_CONNECT(rbac_center);
            rbac_role tmp;
            tmp.role_name = _params[0];
            tmp.read_only = atoi(_params[1].c_str());
            client->add_role(tmp);
            TRH_CLOSE();
        }
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}
void del_role(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 1)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(rbac_center);
            THR_CONNECT(rbac_center);
            client->del_role(atoi(_params[0].c_str()));
            TRH_CLOSE();
        }
    }
    catch (const gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

int main(int argc, char const *argv[])
{
    auto root_menu = std::unique_ptr<cli::Menu>(new cli::Menu("zczh"));

    root_menu->Insert(CLI_MENU_ITEM(show_users), "查看用户");
    root_menu->Insert(CLI_MENU_ITEM(show_permission), "查看权限");
    root_menu->Insert(CLI_MENU_ITEM(show_roles), "查看角色");
    root_menu->Insert(CLI_MENU_ITEM(add_user), "添加用户", {"用户名", "电话", "密码"});
    root_menu->Insert(CLI_MENU_ITEM(del_user), "删除用户", {"编号"});
    root_menu->Insert(CLI_MENU_ITEM(add_role), "添加角色", {"名称", "是否只读:1->是, 0->否"});
    root_menu->Insert(CLI_MENU_ITEM(del_role), "删除角色", {"编号"});
    root_menu->Insert(CLI_MENU_ITEM(add_user_to_role), "用户添加角色", {"角色编号", "用户编号"});
    root_menu->Insert(CLI_MENU_ITEM(del_user_from_role), "用户取消角色", {"角色编号", "用户编号"});
    root_menu->Insert(CLI_MENU_ITEM(add_perm_to_role), "添加角色权限", {"角色编号", "权限编号"});
    root_menu->Insert(CLI_MENU_ITEM(del_perm_from_role), "取消角色权限", {"角色编号", "权限编号"});

    cli::Cli cli(std::move(root_menu));
    cli::LoopScheduler sc;
    cli::CliLocalTerminalSession ss(cli, sc, std::cout);

    sc.Run();
    return 0;
}
