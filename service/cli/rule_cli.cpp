#include "rule_cli.h"
#include "tabulate.hpp"
void bdr(std::ostream &out, std::vector<std::string> _params)
{
    rule_cli tmp;
    out << tmp.make_bdr() << std::endl;
}

void set_zyzl_plugin(std::ostream &out, std::vector<std::string> _params)
{
    if (_params.size() != 2)
    {
        out << "参数错误" << std::endl;
    }
    else
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        try
        {
            running_rule tmp;
            client->get_rule(tmp);
            tmp.zyzl_host = _params[0];
            tmp.zyzl_ssid = _params[1];
            client->set_rule(tmp);
        }
        catch (const gen_exp &e)
        {
            out << e.msg << std::endl;
        }
        TRH_CLOSE();
    }
}

void set_ticket_prefix(std::ostream &out, std::vector<std::string> _params)
{
    if (_params.size() != 1)
    {
        out << "参数错误" << std::endl;
    }
    else
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        try
        {
            running_rule tmp;
            client->get_rule(tmp);
            tmp.date_ticket_prefix = _params[0];
            client->set_rule(tmp);
        }
        catch (const gen_exp &e)
        {
            out << e.msg << std::endl;
        }
        TRH_CLOSE();
    }
}

void auto_call_count(std::ostream &out, std::vector<std::string> _params)
{
    if (_params.size() != 1)
    {
        out << "参数错误" << std::endl;
    }
    else
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        try
        {
            running_rule tmp;
            client->get_rule(tmp);
            tmp.auto_call_count = atoi(_params[0].c_str());
            client->set_rule(tmp);
        }
        catch (const gen_exp &e)
        {
            out << e.msg << std::endl;
        }
        TRH_CLOSE();
    }
}

void weight_turn_set(std::ostream &out, std::vector<std::string> _params)
{
    if (_params.size() != 1)
    {
        out << "参数错误" << std::endl;
    }
    else
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        try
        {
            running_rule tmp;
            client->get_rule(tmp);
            tmp.weight_turn = atoi(_params[0].c_str());
            client->set_rule(tmp);
        }
        catch (const gen_exp &e)
        {
            out << e.msg << std::endl;
        }
        TRH_CLOSE();
    }
}

void call_time_out(std::ostream &out, std::vector<std::string> _params)
{
    if (_params.size() != 1)
    {
        out << "参数错误" << std::endl;
    }
    else
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        try
        {
            running_rule tmp;
            client->get_rule(tmp);
            tmp.call_time_out = atoi(_params[0].c_str());
            client->set_rule(tmp);
        }
        catch (const gen_exp &e)
        {
            out << e.msg << std::endl;
        }
        TRH_CLOSE();
    }
}
static void clear(std::ostream &out, std::vector<std::string> _params)
{
    if (_params.size() != 0)
    {
        out << "参数错误" << std::endl;
    }
    else
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        try
        {
            running_rule tmp;
            client->set_rule(tmp);
        }
        catch (const gen_exp &e)
        {
            out << e.msg << std::endl;
        }
        TRH_CLOSE();
    }
}

std::unique_ptr<cli::Menu> make_rule_cli(const std::string &_menu_name)
{
    auto root_menu = std::unique_ptr<cli::Menu>(new cli::Menu(_menu_name));

    root_menu->Insert(CLI_MENU_ITEM(bdr), "列出配置");
    root_menu->Insert(CLI_MENU_ITEM(auto_call_count), "自动叫号容量", {"容量值"});
    root_menu->Insert(CLI_MENU_ITEM(call_time_out), " 自动过号时间", {"过号时间（分钟）"});
    root_menu->Insert(CLI_MENU_ITEM(set_zyzl_plugin), "设置掌易插件参数", {"主机地址", "调用凭证"});
    root_menu->Insert(CLI_MENU_ITEM(set_ticket_prefix), "设置磅单号前缀", {"前缀"});
    root_menu->Insert(CLI_MENU_ITEM(weight_turn_set), "设置称重轮数", {"轮数"});
    root_menu->Insert(CLI_MENU_ITEM(clear), "清除配置");

    return root_menu;
}
rule_cli::rule_cli() : common_cli(make_rule_cli("rule"), "rule")
{
}

std::string rule_cli::make_bdr()
{
    std::vector<std::string> ret;
    running_rule tmp;
    THR_CALL_BEGIN(config_management);
    client->get_rule(tmp);
    THR_CALL_END();

    if (tmp.auto_call_count != 0)
    {
        ret.push_back("auto_call_count " + std::to_string(tmp.auto_call_count));
    }
    if (tmp.call_time_out != 0)
    {
        ret.push_back("call_time_out " + std::to_string(tmp.call_time_out));
    }
    if (tmp.zyzl_host.length() > 0)
    {
        ret.push_back("set_zyzl_plugin " + tmp.zyzl_host + " " + tmp.zyzl_ssid);
    }
    if (tmp.date_ticket_prefix.length() > 0)
    {
        ret.push_back("set_ticket_prefix " + tmp.date_ticket_prefix);
    }
    if (tmp.weight_turn != 0)
    {
        ret.push_back("weight_turn_set " + std::to_string(tmp.weight_turn));
    }

    return util_join_string(ret, "\n");
}
