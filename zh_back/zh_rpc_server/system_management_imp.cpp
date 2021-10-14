#include "system_management_imp.h"
#include "../zh_database/zh_db_config.h"
#include "zh_rpc_util.h"
#include <iostream>
#include <iosfwd>
#include <fstream>
#include "../zh_raster/lib/zh_raster.h"

#define ZH_RASTER_PORT 30200
#define ZH_SCALE_PORT 30201
#define ZH_ID_READER_PORT 30202
#define ZH_PRINTER_PORT 30203

system_management_handler *system_management_handler::m_inst = nullptr;

bool system_management_handler::reboot_system(const std::string &ssid)
{
    return true;
}
void system_management_handler::current_version(std::string &_return)
{
    _return = "v1.0";
}

void system_management_handler::get_device_config(device_config &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 0);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    std::ifstream config_file("/conf/device/device_config.json", std::ios::in);
    std::istreambuf_iterator<char> beg(config_file), end;
    std::string config_string(beg, end);
    neb::CJsonObject config(config_string);

    auto gate_config = config["gate"];
    auto scale_config = config["scale"];
    for (int i = 0; i < gate_config.GetArraySize(); i++) 
    {
        device_gate_config tmp;
        tmp.entry = gate_config[i]("entry");
        tmp.exit = gate_config[i]("exit");
        tmp.name = gate_config[i]("name");
        tmp.entry_id_reader_ip = gate_config[i]("entry_id_reader_ip");
        _return.gate.push_back(tmp);
    }
    for (int i = 0; i < scale_config.GetArraySize(); i++)
    {
        device_scale_config tmp;
        tmp.entry = scale_config[i]("entry");
        tmp.entry_printer_ip = scale_config[i]("entry_printer_ip");
        tmp.exit = scale_config[i]("exit");
        tmp.exit_printer_ip = scale_config[i]("exit_printer_ip");
        tmp.name = scale_config[i]("name");
        tmp.raster_ip.push_back(scale_config[i]["raster_ip"](0));
        tmp.raster_ip.push_back(scale_config[i]["raster_ip"](1));
        tmp.scale_ip = scale_config[i]("scale_ip");
        _return.scale.push_back(tmp);
    }
}

bool system_management_handler::edit_device_config(const std::string &ssid, const device_config &config)
{
    bool ret = false;

    auto opt_user = zh_rpc_util_get_online_user(ssid, 0);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    std::ofstream config_file("/conf/device/device_config.json", std::ios::out);
    neb::CJsonObject tmp;
    tmp.AddEmptySubArray("gate");
    tmp.AddEmptySubArray("scale");
    for (auto &itr:config.gate)
    {
        neb::CJsonObject gate;
        gate.Add("entry", itr.entry);
        gate.Add("exit", itr.exit);
        gate.Add("entry_id_reader_ip", itr.entry_id_reader_ip);
        gate.Add("name", itr.name);
        tmp["gate"].Add(gate);
    }

    for (auto &itr:config.scale)
    {
        neb::CJsonObject scale;
        scale.Add("entry", itr.entry);
        scale.Add("exit", itr.exit);
        scale.Add("name", itr.name);
        scale.Add("scale_ip", itr.scale_ip);
        scale.Add("entry_printer_ip", itr.entry_printer_ip);
        scale.Add("exit_printer_ip", itr.exit_printer_ip);
        scale.AddEmptySubArray("raster_ip");
        scale["raster_ip"].Add(itr.raster_ip[0]);
        scale["raster_ip"].Add(itr.raster_ip[1]);
        tmp["scale"].Add(scale);
    }

    config_file << tmp.ToFormattedString();

    return true;
}

bool system_management_handler::raster_is_block(const std::string &raster_ip)
{
    return raster_was_block(raster_ip, ZH_RASTER_PORT);
}