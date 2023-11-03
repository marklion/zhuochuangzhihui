#include "../lib/rpc_include.h"

#define CLI_MENU_ITEM(x) #x,[](std::ostream &out, std::vector<std::string> _params){x(out, _params);}

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
        out << "params given wrong" << std::endl;
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
        std::vector<std::string> tmp;
        client->get_all_permission(tmp);
        for (auto &itr : tmp)
        {
            out << itr << std::endl;
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
        out << "ID\tname\tusers\tpermissions" << std::endl;
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
                perms.push_back("模块:" + su);
            }
            for (auto &su : itr.author_resouces)
            {
                perms.push_back("资源:" + su);
            }
            role_content.push_back(util_join_string(users));
            role_content.push_back(util_join_string(perms));
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
            out << "params given wrong" << std::endl;
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
            out << "params given wrong" << std::endl;
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

int main(int argc, char const *argv[])
{
    auto root_menu = std::unique_ptr<cli::Menu>(new cli::Menu("zczh"));

    root_menu->Insert(CLI_MENU_ITEM(show_users));
    root_menu->Insert(CLI_MENU_ITEM(add_user), "add user to rbac", {"name", "phone", "pwd"});
    root_menu->Insert(CLI_MENU_ITEM(show_permission));
    root_menu->Insert(CLI_MENU_ITEM(show_roles));
    root_menu->Insert(CLI_MENU_ITEM(add_user_to_role), "add user to role", {"user id", "role id"});
    root_menu->Insert(CLI_MENU_ITEM(del_user_from_role), "del user from role", {"user id", "role id"});

    cli::Cli cli(std::move(root_menu));
    cli::LoopScheduler sc;
    cli::CliLocalTerminalSession ss(cli, sc, std::cout);

    sc.Run();
    return 0;
}
