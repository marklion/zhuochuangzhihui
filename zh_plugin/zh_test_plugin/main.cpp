#include <unistd.h>
#include <iostream>
#include "clipp.h"
#include "../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#include "../../zh_back/zh_tdf/tdf_include.h"

#define PLUGIN_CONF_FILE "/plugin/zh_test_plugin/conf/plugin.json"
static tdf_log g_log("test_plugin", "/plugin/audit.log", "/plugin/audit.log");

int main(int argc, char **argv)
{
    int iret = -1;
    std::string cmd;
    std::string order_number;
    std::string event_name;
    std::string key;
    std::string json_string;
    bool json_obj = false;
    bool cmd_test = false;
    using namespace clipp;
    auto cli = ((command("set").set(cmd) & required("-k") & value("json key", key) & value("json value", json_string) & option("-o").set(json_obj)) |
                command("init").set(cmd) |
                command("enabled").set(cmd) |
                (command("proc_event").set(cmd) & required("-c") & value("event_name", event_name) & value("order_number", order_number) & option("-t").set(cmd_test)));
    if (!parse(argc, argv, cli))
    {
        std::cerr << "Usage:\n"
                  << usage_lines(cli, argv[0])
                  << '\n';
    }

    if (cmd == "enabled")
    {
        if (zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("enabled") == "true")
        {
            iret = 0;
        }
    }
    else if (cmd == "init")
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "enabled", "true");
        iret = 0;
    }
    else if (cmd == "proc_event")
    {
        if (event_name == "create_order")
        {
            if (cmd_test)
            {
                iret = 0;
            }
            else
            {
                g_log.log("proc create order:%s", order_number.c_str());
                iret = -1;
            }
        }
    }

    return iret;
}
