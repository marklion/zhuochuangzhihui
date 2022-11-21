#include <unistd.h>
#include <iostream>
#include "clipp.h"
#include "../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include <functional>
#include <thrift/transport/THttpClient.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include "lib/zh_zyzl.h"

#define PLUGIN_CONF_FILE "/plugin/zh_zyzl/conf/plugin.json"
static tdf_log g_log("zyzl", "/plugin/audit.log", "/plugin/audit.log");

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

struct weight_detail {
    std::string p_time;
    std::string m_time;
    double p_weight;
    double m_weight;
    std::string seal_no;
    std::string ticket_no;
};

static weight_detail get_weight_detail(const std::string &_order_number)
{
    weight_detail ret;
    using namespace ::apache::thrift;
    using namespace ::apache::thrift::protocol;
    using namespace ::apache::thrift::transport;
    THR_DEF_CIENT(vehicle_order_center);
    THR_CONNECT(vehicle_order_center);
    vehicle_order_detail tmp;
    client->get_order_detail(tmp, "", _order_number);
    TRH_CLOSE();

    ret.p_time = tmp.p_time;
    ret.m_time = tmp.m_time;
    ret.p_weight = tmp.basic_info.p_weight;
    ret.m_weight = tmp.basic_info.m_weight;
    ret.ticket_no = tmp.basic_info.order_number;
    ret.seal_no = tmp.basic_info.seal_no;
    return ret;
}

static bool push_weight_wapper(const std::string &_plate, const std::string &_order_number)
{
    bool ret = false;

    auto wd = get_weight_detail(_order_number);

    return ZH_ZYZL_push_weight(_plate, wd.p_time, wd.m_time, wd.p_weight, wd.m_weight, wd.m_weight - wd.p_weight, wd.ticket_no, wd.seal_no);
}

static bool call_vehicle_wapper(const std::string &_plate, const std::string &_order_number)
{
    return ZH_ZYZL_call_vehicle(_plate);
}
static bool push_p_wapper(const std::string &_plate, const std::string &_order_number)
{
    return ZH_ZYZL_push_p(_plate);
}

static std::map<std::string, std::function<bool(const std::string &, const std::string &)>> g_proc_map =
    {
        {"call_driver", call_vehicle_wapper},
        {"vehicle_p_weight", push_p_wapper},
        {"vehicle_m_weight", push_weight_wapper}};

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
                (command("proc_event").set(cmd) & required("-c") & value("event_name", event_name) & value("order_number", order_number) & option("-t").set(cmd_test)) |
                command("sync_plan").set(cmd) |
                (command("pull_ticket").set(cmd) & value("vehicle_number", vehicle_number)) |
                (command("call_vehicle").set(cmd) & value("vehicle_number", vehicle_number)));
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
    else if (cmd == "sync_plan")
    {
        std::istreambuf_iterator<char> beg(std::cin), end;
        std::string config_string(beg, end);
        neb::CJsonObject config(config_string);
        std::list<zy_sync_plan_data> tmp;
        for (auto i = 0; i < config.GetArraySize(); i++)
        {
            tmp.push_back(json_to_struct_plan(config[i]));
        }
        ZH_ZYZL_sync_plans(tmp);
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
        iret = 0;
    }
    else if (cmd == "pull_ticket")
    {
        std::cout << ZH_ZYZL_pull_ticket(vehicle_number) << std::endl;
        iret = 0;
    }
    else if (cmd == "call_vehicle")
    {
        ZH_ZYZL_call_vehicle(vehicle_number);
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
                    auto &proc_func = itr->second;

                    if (proc_func(vehicle_number, order_number))
                    {
                        iret = 0;
                    }
                }
            }
        }
    }

    return iret;
}
