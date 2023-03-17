#include "lib/zh_led_plan_call.h"
#include <unistd.h>
#include <iostream>
#include "../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include <functional>
#include <thrift/transport/THttpClient.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/protocol/TMultiplexedProtocol.h>

#define PLUGIN_CONF_FILE "/plugin/zh_led_plan_call/conf/plugin.json"
static tdf_log g_log("led_plan_call", "/plugin/device.log", "/plugin/device.log");

#define THR_DEF_CIENT(x) x##Client *client = nullptr
#define THR_CONNECT(x) std::shared_ptr<TTransport> transport(new THttpClient("localhost", 8123, "/zh_rpc"));std::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport)); transport->open();  std::shared_ptr<TMultiplexedProtocol> mp(new TMultiplexedProtocol(protocol, #x)); client = new x##Client(mp)
#define TRH_CLOSE() transport->close()

static std::string get_vehicle_number_by_order_number(const std::string &_order_number)
{
    using namespace ::apache::thrift;
    using namespace ::apache::thrift::protocol;
    using namespace ::apache::thrift::transport;
    THR_DEF_CIENT(vehicle_order_center);
    THR_CONNECT(vehicle_order_center);
    std::string ret;
    vehicle_order_detail tmp;
    client->get_order_detail(tmp, "", _order_number);
    ret = tmp.basic_info.main_vehicle_number;
    TRH_CLOSE();

    return ret;
}

static std::map<std::string, std::function<void(const std::string &)>> g_proc_map =
    {
        {"confirm_order", zh_led_plan_call_add_plan_vehicle},
        {"order_close", zh_led_plan_call_del_plan_vehicle},
        {"call_driver", zh_led_plan_call_add_call_vehicle},
        {"vehicle_p_weight", zh_led_plan_call_del_call_vehicle}};

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
    bool cmd_test = false;
    using namespace clipp;
    auto cli = ((command("get").set(cmd) & required("-k") & value("json key", key)) |
                (command("set").set(cmd) & required("-k") & value("json key", key) & value("json value", json_string) & option("-o").set(json_obj)) |
                command("init").set(cmd) |
                command("enabled").set(cmd) |
                command("show").set(cmd) |
                command("refresh").set(cmd) |
                (command("proc_event").set(cmd) & required("-c") & value("event_name", event_name) & value("order_number", order_number) & option("-t").set(cmd_test)) |
                (command("push_call").set(cmd) & value("vehicle_number", vehicle_number)) |
                (command("del_call").set(cmd) & value("vehicle_number", vehicle_number)) |
                (command("push_plan").set(cmd) & value("vehicle_number", vehicle_number)) |
                (command("del_plan").set(cmd) & value("vehicle_number", vehicle_number)));
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
    else if (cmd == "refresh")
    {
        if (zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("enabled") == "true")
        {
            zh_led_plan_call_show();
            iret = 0;
        }
    }
    else if (cmd == "init")
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "enabled", "true");
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "cur_plan_page", "0");
    }
    else if (cmd == "show")
    {
        if (zh_led_plan_call_show())
        {
            iret = 0;
        }
    }
    else if (cmd == "push_plan")
    {
        zh_led_plan_call_add_plan_vehicle(vehicle_number);
        iret = 0;
    }
    else if (cmd == "push_call")
    {
        zh_led_plan_call_add_call_vehicle(vehicle_number);
        iret = 0;
    }
    else if (cmd == "del_plan")
    {
        zh_led_plan_call_del_plan_vehicle(vehicle_number);
        iret = 0;
    }
    else if (cmd == "del_call")
    {
        zh_led_plan_call_del_call_vehicle(vehicle_number);
        iret = 0;
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
                auto vehicle_number = get_vehicle_number_by_order_number(order_number);
                if (vehicle_number.length() > 0)
                {
                    itr->second(vehicle_number);
                    iret = 0;
                }
            }
        }
    }

    return iret;
}
