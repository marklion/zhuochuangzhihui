#include "zh_led_plan_call.h"
#include <modbus/modbus.h>
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#include <iconv.h>
#define PLUGIN_CONF_FILE "/plugin/zh_led_plan_call/conf/plugin.json"

static tdf_log g_log("led_plan_call", "/plugin/device.log", "/plugin/device.log");
int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}
int u2g(char *inbuf, int inlen, char *outbuf, int outlen)
{
    return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}
std::string utf2gbk(const std::string &_gbk)
{
    char in_buff[2048] = {0};
    char out_buff[2048] = {0};
    strcpy(in_buff, _gbk.c_str());
    u2g(in_buff, strlen(in_buff), out_buff, sizeof(out_buff));
    return std::string(out_buff);
}
static bool _zone_write_string(modbus_t *mtx, int _zone_no, const std::string &_content)
{
    bool ret = false;

    unsigned short content_buff[8] = {0};
    auto gbk_content = utf2gbk(_content);
    strncpy((char *)content_buff, gbk_content.c_str(), sizeof(content_buff));
    for (auto &itr : content_buff)
    {
        unsigned char tmp = (unsigned char)(itr >> 8);
        itr = itr << 8;
        itr |= tmp;
    }
    if (8 == modbus_write_registers(mtx, (_zone_no - 1) * 8, 8, content_buff))
    {
        ret = true;
    }
    return ret;
}

static void _add_vehicle_in_que(const std::string &_vehicle, const std::string &_type_key)
{
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto vehicles = config[_type_key];
    vehicles.Add(_vehicle);

    zh_plugin_conf_set_config(PLUGIN_CONF_FILE, _type_key, vehicles);
}

static void _del_vehicle_from_que(const std::string &_vehicle, const std::string &_type_key)
{
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto vehicles = config[_type_key];
    neb::CJsonObject new_vehicles;
    for (auto i = 0; i < vehicles.GetArraySize(); i++)
    {
        g_log.log("found vehicle:%s", vehicles(i).c_str());
        if (_vehicle != vehicles(i))
        {
            new_vehicles.Add(vehicles(i));
        }
    }
    zh_plugin_conf_set_config(PLUGIN_CONF_FILE, _type_key, new_vehicles);
}

void zh_led_plan_call_del_plan_vehicle(const std::string &_vehicle)
{
    _del_vehicle_from_que(_vehicle, "plans");
}

void zh_led_plan_call_del_call_vehicle(const std::string &_vehicle)
{
    _del_vehicle_from_que(_vehicle, "calls");
}

void zh_led_plan_call_add_call_vehicle(const std::string &_vehicle)
{
    _add_vehicle_in_que(_vehicle, "calls");
}
void zh_led_plan_call_add_plan_vehicle(const std::string &_vehicle)
{
    _add_vehicle_in_que(_vehicle, "plans");
}

static std::list<std::string> _get_cur_calls()
{
    std::list<std::string> ret;
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto cur_page = atoi(config("cur_call_page").c_str());
    auto call_count = atoi(config("call_count").c_str());
    int i = cur_page * call_count;
    while (i < config["calls"].GetArraySize())
    {
        ret.push_back(config["calls"](i++));
        if (ret.size() >= call_count)
        {
            break;
        }
    }
    for (auto j = ret.size(); j < call_count; j++)
    {
        ret.push_back("");
    }
    if (i < config["calls"].GetArraySize())
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "cur_call_page", std::to_string(cur_page + 1));
    }
    else
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "cur_call_page", std::to_string(0));
    }

    return ret;
}

static std::list<std::string> _get_cur_plans()
{
    std::list<std::string> ret;

    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto cur_page = atoi(config("cur_plan_page").c_str());
    auto plan_count = atoi(config("plan_count").c_str());
    int i = cur_page * plan_count;
    while (i < config["plans"].GetArraySize())
    {
        ret.push_back(config["plans"](i++));
        if (ret.size() >= plan_count)
        {
            break;
        }
    }
    for (auto j = ret.size(); j < plan_count; j++)
    {
        ret.push_back("");
    }
    if (i < config["plans"].GetArraySize())
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "cur_plan_page", std::to_string(cur_page + 1));
    }
    else
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "cur_plan_page", std::to_string(0));
    }

    return ret;
}

bool zh_led_plan_call_show()
{
    if (fork() == 0)
    {
        sleep(20);
        auto mtx = modbus_new_tcp(zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("led_ip").c_str(), MODBUS_TCP_DEFAULT_PORT);
        if (mtx)
        {
            if (0 == modbus_connect(mtx))
            {
                auto cur_plans = _get_cur_plans();

                int zone_id = 1;
                for (auto &itr : cur_plans)
                {
                    _zone_write_string(mtx, zone_id++, itr);
                }
                auto cur_calls = _get_cur_calls();
                for (auto &itr : cur_calls)
                {
                    _zone_write_string(mtx, zone_id++, itr);
                }

                modbus_close(mtx);
            }
            else
            {
                g_log.err("failed to connect modbus:%s", modbus_strerror(errno));
            }
            modbus_free(mtx);
        }
        else
        {
            g_log.err("failed to open tcp modbus:%s", modbus_strerror(errno));
        }
        auto new_cmd = "/plugin/zh_led_plan_call/bin/zh_led_plan_call_plugin show";
        system(new_cmd);
    }

    return true;
}