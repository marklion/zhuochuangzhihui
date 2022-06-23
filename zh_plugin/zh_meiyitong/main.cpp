#include <unistd.h>
#include <iostream>
#include "clipp.h"
#include "../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#include "lib/zh_meiyitong.h"
#define PLUGIN_CONF_FILE "/plugin/zh_meiyitong/conf/plugin.json"

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
    auto cli = (command("fetch_dest").set(cmd) |
                (command("get").set(cmd) & required("-k") & value("json key", key)) |
                (command("set").set(cmd) & required("-k") & value("json key", key) & value("json value", json_string) & option("-o").set(json_obj)) |
                command("init").set(cmd));
    if (!parse(argc, argv, cli))
    {
        std::cerr << "Usage:\n"
                  << usage_lines(cli, argv[0])
                  << '\n';
    }

    if (cmd == "fetch_dest")
    {
        if (ZH_MEIYITONG_get_dest_addr())
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
    else if (cmd == "init")
    {
        iret = 0;
    }

    return iret;
}
