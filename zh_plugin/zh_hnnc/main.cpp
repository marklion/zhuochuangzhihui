#include "lib/zh_hnnc.h"
#include <unistd.h>
#include <iostream>
#include "clipp.h"
#include "../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#define PLUGIN_CONF_FILE "/plugin/zh_hnnc/conf/plugin.json"

int main(int argc, char **argv)
{
    int iret = -1;
    std::string cmd;
    std::string customer_code;
    std::string vehicle_count;
    std::string inv_code;
    std::string json_string;
    std::string key;
    bool json_obj = false;
    using namespace clipp;
    auto cli = (command("fetchCorp").set(cmd) |
                (command("get").set(cmd) & required("-k") & value("json key", key)) |
                (command("set").set(cmd) & required("-k") & value("json key", key) & value("json value", json_string) & option("-o").set(json_obj)) |
                command("init").set(cmd) |
                command("fetchCust").set(cmd) |
                command("push_req").set(cmd) & value("json_string", json_string) |
                (command("valid_balance").set(cmd) & value("cust_code", customer_code) & value("inv_code", inv_code)) & value("vehicle_count", vehicle_count) |
                (command("fetchOrder").set(cmd) & value("Custcode", customer_code)));
    if (!parse(argc, argv, cli))
    {
        std::cerr << "Usage:\n"
                  << usage_lines(cli, argv[0])
                  << '\n';
    }

    if (cmd == "fetchCorp")
    {
        if (ZH_HNNC_fetch_cor_info())
        {
            iret = 0;
        }
    }
    else if (cmd == "get")
    {
        std::string get_value;
        auto config_value = zh_plugin_conf_get_config(PLUGIN_CONF_FILE)(key);
        if (!config_value.empty())
        {
            get_value = config_value;
        }
        else
        {
            get_value = zh_plugin_conf_get_config(PLUGIN_CONF_FILE)[key].ToString();
        }
        std::cout << get_value << std::endl;
        iret = 0;
    }
    else if (cmd == "set")
    {
        if (json_obj)
        {
            zh_plugin_conf_set_config(PLUGIN_CONF_FILE, key, neb::CJsonObject(json_string));
        }
        else
        {
            zh_plugin_conf_set_config(PLUGIN_CONF_FILE, key, json_string);
        }
        iret = 0;
    }
    else if (cmd == "fetchCust")
    {
        if (ZH_HNNC_fetch_customer_info())
        {
            iret = 0;
        }
    }
    else if (cmd == "fetchOrder")
    {
        if (ZH_HNNC_fetch_order_info(customer_code))
        {
            iret = 0;
        }
    }
    else if (cmd == "valid_balance")
    {
        if (ZH_HNNC_valid_balance(customer_code, inv_code, atoi(vehicle_count.c_str())))
        {
            iret = 0;
        }
    }
    else if (cmd == "push_req")
    {
        if (ZH_HNNC_push_req(json_string))
        {
            iret = 0;
        }
    }
    else if (cmd == "init")
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "remote_url", "http://www.hnjt.top");
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "username", "mhg");
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "password", "mhg123456");
    }

    return iret;
}
