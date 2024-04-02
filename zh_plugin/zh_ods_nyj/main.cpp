#include "lib/zh_ods_nyj.h"
#include <unistd.h>
#include <iostream>
#include "clipp.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include <functional>
#include <thrift/transport/THttpClient.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/protocol/TMultiplexedProtocol.h>

static tdf_log g_log("zh_ods_nyj", "/plugin/audit.log", "/plugin/audit.log");

static void emp_proc_func(const std::string &_order_number)
{
    g_log.log("proc event %s", _order_number.c_str());
}

static std::map<std::string, std::function<bool(const std::string &)>> g_proc_map =
    {
        {"vehicle_p_weight", proc_p_weight},
        {"vehicle_m_weight", proc_m_weight}};

int main(int argc, char **argv)
{
    int iret = -1;
    std::string cmd;
    std::string json_string;
    std::string key;
    bool json_obj = false;
    std::string vehicle_number;
    std::string order_number;
    std::string event_name;
    std::string plan_date;
    std::string file_name;
    std::string begin_date;
    std::string end_date;
    std::string trans_company;
    bool cmd_test = false;
    using namespace clipp;
    auto cli = ((command("get").set(cmd) & required("-k") & value("json key", key)) |
                (command("set").set(cmd) & required("-k") & value("json key", key) & value("json value", json_string) & option("-o").set(json_obj)) |
                command("init").set(cmd) |
                command("refresh").set(cmd) |
                command("enabled").set(cmd) |
                (command("proc_event").set(cmd) & required("-c") & value("event_name", event_name) & value("order_number", order_number) & option("-t").set(cmd_test)));
    if (!parse(argc, argv, cli))
    {
        std::cerr << "Usage:\n"
                  << usage_lines(cli, argv[0])
                  << '\n';
    }

    if (cmd == "get")
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
    else if (cmd == "enabled")
    {
        if (zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("enabled") == "true")
        {
            iret = 0;
        }
    }
    else if (cmd == "init")
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "enabled", "true");
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "remote_url", "http://58.18.38.116:8811");
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "access_code", "4hBY03s2qe6m7mTY");
    }
    else if (cmd == "proc_event")
    {
        auto itr = g_proc_map.find(event_name);
        if (itr != g_proc_map.end())
        {
            if (cmd_test)
            {
                iret = 0;
            }
            else
            {
                if (itr->second(order_number))
                {
                    iret = 0;
                }
            }
        }
        iret = 0;
    }

    return iret;
}
