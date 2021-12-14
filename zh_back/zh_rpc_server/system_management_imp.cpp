#include "system_management_imp.h"
#include "../zh_database/zh_db_config.h"
#include "zh_rpc_util.h"
#include <iostream>
#include <iosfwd>
#include <fstream>
#include "../zh_raster/lib/zh_raster.h"
#include "../zh_id_reader/lib/zh_id_reader.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"
#include "../zh_scale/lib/zh_scale.h"
#include "../zh_printer/lib/zh_printer.h"
#include "vehicle_order_center_imp.h"


system_management_handler *system_management_handler::m_inst = nullptr;

bool system_management_handler::reboot_system(const std::string &ssid)
{
    return true;
}
void system_management_handler::current_version(std::string &_return)
{
    _return = "v1.0";
}

void system_management_handler::internal_get_device_config(device_config &_return)
{
    std::ifstream config_file("/conf/device/device_config.json", std::ios::in);
    std::istreambuf_iterator<char> beg(config_file), end;
    std::string config_string(beg, end);
    neb::CJsonObject config(config_string);

    auto gate_config = config["gate"];
    auto scale_config = config["scale"];
    for (int i = 0; i < gate_config.GetArraySize(); i++)
    {
        device_gate_config tmp;
        tmp.name = gate_config[i]("name");
        tmp.entry_id_reader_ip = gate_config[i]("entry_id_reader_ip");
        tmp.exit_id_reader_ip = gate_config[i]("exit_id_reader_ip");
        tmp.entry_config.cam_ip = gate_config[i]("entry_cam_ip");
        tmp.entry_config.led_ip = gate_config[i]("entry_led_ip");
        tmp.exit_config.cam_ip = gate_config[i]("exit_cam_ip");
        tmp.exit_config.led_ip = gate_config[i]("exit_led_ip");
        tmp.entry_qr_ip = gate_config[i]("entry_qr_ip");
        tmp.exit_qr_ip = gate_config[i]("exit_qr_ip");
        gate_config[i].Get("entry_need_id", tmp.entry_need_id);
        gate_config[i].Get("exit_need_id", tmp.exit_need_id);
        gate_config[i].Get("entry_need_qr", tmp.entry_need_qr);
        gate_config[i].Get("exit_need_qr", tmp.exit_need_qr);
        _return.gate.push_back(tmp);
    }
    for (int i = 0; i < scale_config.GetArraySize(); i++)
    {
        device_scale_config tmp;
        tmp.entry_printer_ip = scale_config[i]("entry_printer_ip");
        tmp.exit_printer_ip = scale_config[i]("exit_printer_ip");
        tmp.name = scale_config[i]("name");
        tmp.raster_ip.push_back(scale_config[i]["raster_ip"](0));
        tmp.raster_ip.push_back(scale_config[i]["raster_ip"](1));
        tmp.scale_ip = scale_config[i]("scale_ip");
        tmp.entry_id_reader_ip = scale_config[i]("entry_id_reader_ip");
        tmp.exit_id_reader_ip = scale_config[i]("exit_id_reader_ip");
        tmp.entry_config.cam_ip = scale_config[i]("entry_cam_ip");
        tmp.entry_config.led_ip = scale_config[i]("entry_led_ip");
        tmp.exit_config.cam_ip = scale_config[i]("exit_cam_ip");
        tmp.exit_config.led_ip = scale_config[i]("exit_led_ip");
        tmp.entry_qr_ip = scale_config[i]("entry_qr_ip");
        tmp.exit_qr_ip = scale_config[i]("exit_qr_ip");
        scale_config[i].Get("need_id", tmp.need_id);
        scale_config[i].Get("need_qr", tmp.need_qr);
        _return.scale.push_back(tmp);
    }
}
void system_management_handler::get_device_config(device_config &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 0);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    internal_get_device_config(_return);
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
    vehicle_order_center_handler::gsm_map.clear();
    vehicle_order_center_handler::ssm_map.clear();
    tmp.AddEmptySubArray("gate");
    tmp.AddEmptySubArray("scale");
    for (auto &itr:config.gate)
    {
        neb::CJsonObject gate;
        gate.Add("entry_id_reader_ip", itr.entry_id_reader_ip);
        gate.Add("exit_id_reader_ip", itr.exit_id_reader_ip);
        gate.Add("name", itr.name);
        gate.Add("entry_cam_ip", itr.entry_config.cam_ip);
        gate.Add("entry_led_ip", itr.entry_config.led_ip);
        gate.Add("exit_cam_ip", itr.exit_config.cam_ip);
        gate.Add("exit_led_ip", itr.exit_config.led_ip);
        gate.Add("entry_need_id", itr.entry_need_id, itr.entry_need_id);
        gate.Add("exit_need_id", itr.exit_need_id, itr.exit_need_id);
        gate.Add("entry_need_qr", itr.entry_need_qr, itr.entry_need_qr);
        gate.Add("exit_need_qr", itr.exit_need_qr, itr.exit_need_qr);
        gate.Add("entry_qr_ip", itr.entry_qr_ip);
        gate.Add("exit_qr_ip", itr.exit_qr_ip);
        tmp["gate"].Add(gate);
        vehicle_order_center_handler::gsm_map[itr.entry_config.cam_ip] = std::make_shared<gate_state_machine>(itr.entry_config.cam_ip, itr.entry_id_reader_ip,itr.entry_qr_ip, true);
        vehicle_order_center_handler::gsm_map[itr.entry_config.cam_ip]->ctrl_policy.set_policy(itr.entry_need_id, itr.entry_need_qr);
        vehicle_order_center_handler::gsm_map[itr.exit_config.cam_ip] = std::make_shared<gate_state_machine>(itr.exit_config.cam_ip, itr.exit_id_reader_ip,itr.exit_qr_ip, false);
        vehicle_order_center_handler::gsm_map[itr.exit_config.cam_ip]->ctrl_policy.set_policy(itr.exit_need_id, itr.exit_need_qr);
    }

    for (auto &itr : config.scale)
    {
        neb::CJsonObject scale;
        scale.Add("name", itr.name);
        scale.Add("scale_ip", itr.scale_ip);
        scale.Add("entry_printer_ip", itr.entry_printer_ip);
        scale.Add("exit_printer_ip", itr.exit_printer_ip);
        scale.AddEmptySubArray("raster_ip");
        scale["raster_ip"].Add(itr.raster_ip[0]);
        scale["raster_ip"].Add(itr.raster_ip[1]);
        scale.Add("entry_id_reader_ip", itr.entry_id_reader_ip);
        scale.Add("exit_id_reader_ip", itr.exit_id_reader_ip);
        scale.Add("entry_cam_ip", itr.entry_config.cam_ip);
        scale.Add("entry_led_ip", itr.entry_config.led_ip);
        scale.Add("exit_cam_ip", itr.exit_config.cam_ip);
        scale.Add("exit_led_ip", itr.exit_config.led_ip);
        scale.Add("entry_qr_ip", itr.entry_qr_ip);
        scale.Add("exit_qr_ip", itr.exit_qr_ip);
        scale.Add("need_id", itr.need_id, itr.need_id);
        scale.Add("need_qr", itr.need_qr, itr.need_qr);
        tmp["scale"].Add(scale);
        vehicle_order_center_handler::ssm_map[itr.name] = std::make_shared<scale_state_machine>(itr);
        vehicle_order_center_handler::ssm_map[itr.name]->ctrl_policy.set_policy(itr.need_id, itr.need_qr);
    }

    config_file << tmp.ToFormattedString();

    return true;
}

bool system_management_handler::raster_is_block(const std::string &raster_ip)
{
    return raster_was_block(raster_ip, ZH_RASTER_PORT);
}

bool system_management_handler::print_content(const std::string &printer_ip, const std::string &content, const std::string &qr_code)
{
    tdf_log tmp_log("printer " + printer_ip);
    tmp_log.log("print:content:%s, qr_code:%s", content.c_str(), qr_code.c_str());
    zh_printer_dev tmp(printer_ip);
    tmp.print_string(content);
    if (qr_code.length() > 0)
    {
        tmp.print_qr(qr_code);
    }

    return true;
}

void system_management_handler::read_id_no(std::string &_return, const std::string &id_reader_ip)
{
    _return = zh_read_id_no(id_reader_ip, ZH_ID_READER_PORT);
}

bool system_management_handler::ctrl_gate(const std::string &road_ip, const int64_t cmd)
{
    return zh_hk_ctrl_gate(road_ip, (zh_hk_gate_control_cmd)cmd);
}

bool system_management_handler::ctrl_led(const std::string &gate_code, const std::string &content)
{
    return zh_hk_ctrl_led(gate_code, content);
}
bool system_management_handler::ctrl_voice(const std::string &gate_code, const std::string &content)
{
    return zh_hk_ctrl_voice(gate_code, content);
}
road_status system_management_handler::get_status_by_road(const std::string &_road)
{
    road_status ret;
    if (0 == pthread_mutex_lock(&m_road_status_map_lock))
    {
        ret = m_road_status_map[_road];
        pthread_mutex_unlock(&m_road_status_map_lock);
    }
    else
    {
        tdf_log tmp("road_status");
        tmp.err("failed to lock status map");
    }

    return ret;
}
void system_management_handler::set_status_by_road(const std::string &_road, road_status &status)
{
    if (0 == pthread_mutex_lock(&m_road_status_map_lock))
    {
        m_road_status_map[_road] = status;
        pthread_mutex_unlock(&m_road_status_map_lock);
    }
    else
    {
        tdf_log tmp("road_status");
        tmp.err("failed to lock status map");
    }
}

void system_management_handler::get_road_status(road_status &_return, const std::string &gate_code)
{
    auto status = get_status_by_road(gate_code);
    _return = status;
}

double system_management_handler::read_scale(const std::string &scale_ip)
{
    return get_current_weight(scale_ip, ZH_SCALE_PORT);
}