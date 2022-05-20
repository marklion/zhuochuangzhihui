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
#include <fstream>

system_management_handler *system_management_handler::m_inst = nullptr;

bool system_management_handler::reboot_system(const std::string &ssid)
{
    if (!zh_rpc_util_get_online_user(ssid, 0))
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    _exit(-1);
}
void system_management_handler::current_version(std::string &_return)
{
    std::ifstream version_file("/conf/version.txt");
    std::getline(version_file, _return);
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
        gate_config[i].Get("entry_nvr_ip", tmp.entry_nvr_ip);
        gate_config[i].Get("exit_nvr_ip", tmp.exit_nvr_ip);
        gate_config[i].Get("entry_channel", tmp.entry_channel);
        gate_config[i].Get("exit_channel", tmp.exit_channel);
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
        tmp.scale_brand = scale_config[i]("scale_brand");
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
        scale_config[i].Get("entry_nvr_ip", tmp.entry_nvr_ip);
        scale_config[i].Get("exit_nvr_ip", tmp.exit_nvr_ip);
        scale_config[i].Get("entry_channel", tmp.entry_channel);
        scale_config[i].Get("exit_channel", tmp.exit_channel);
        scale_config[i].Get("coefficient", tmp.coefficient);
        _return.scale.push_back(tmp);
    }
    config.Get("auto_order", _return.auto_order);
}
void system_management_handler::get_device_config(device_config &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 2);
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
    tmp.AddEmptySubArray("gate");
    tmp.AddEmptySubArray("scale");
    for (auto &itr : config.gate)
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
        gate.Add("entry_nvr_ip", itr.entry_nvr_ip);
        gate.Add("exit_nvr_ip", itr.exit_nvr_ip);
        gate.Add("entry_channel", itr.entry_channel);
        gate.Add("exit_channel", itr.exit_channel);
        tmp["gate"].Add(gate);
    }

    for (auto &itr : config.scale)
    {
        neb::CJsonObject scale;
        scale.Add("name", itr.name);
        scale.Add("scale_ip", itr.scale_ip);
        scale.Add("scale_brand", itr.scale_brand);
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
        scale.Add("entry_nvr_ip", itr.entry_nvr_ip);
        scale.Add("exit_nvr_ip", itr.exit_nvr_ip);
        scale.Add("entry_channel", itr.entry_channel);
        scale.Add("exit_channel", itr.exit_channel);
        scale.Add("coefficient", itr.coefficient);
        tmp["scale"].Add(scale);
    }
    tmp.Add("auto_order", config.auto_order, config.auto_order);
    config_file << tmp.ToFormattedString();
    config_file.close();

    vehicle_order_center_handler::gsm_map.clear();
    vehicle_order_center_handler::ssm_map.clear();
    for (auto &itr:config.gate)
    {
        vehicle_order_center_handler::gsm_map[itr.entry_config.cam_ip] = std::make_shared<gate_state_machine>(itr.entry_config.cam_ip, itr.entry_id_reader_ip, itr.entry_qr_ip, true);
        vehicle_order_center_handler::gsm_map[itr.entry_config.cam_ip]->ctrl_policy.set_policy(itr.entry_need_id, itr.entry_need_qr);
        vehicle_order_center_handler::gsm_map[itr.exit_config.cam_ip] = std::make_shared<gate_state_machine>(itr.exit_config.cam_ip, itr.exit_id_reader_ip, itr.exit_qr_ip, false);
        vehicle_order_center_handler::gsm_map[itr.exit_config.cam_ip]->ctrl_policy.set_policy(itr.exit_need_id, itr.exit_need_qr);
    }
    for (auto &itr:config.scale)
    {
        vehicle_order_center_handler::ssm_map[itr.name] = std::make_shared<scale_state_machine>(itr);
        vehicle_order_center_handler::ssm_map[itr.name]->ctrl_policy.set_policy(itr.need_id, itr.need_qr);
    }


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
    _return = id_result[id_reader_ip];
}

bool system_management_handler::ctrl_gate(const std::string &road_ip, const int64_t cmd)
{
    return zh_hk_ctrl_gate(road_ip, (zh_hk_gate_control_cmd)cmd);
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
    _return.coming_vehicle = cam_result[gate_code];
}

double system_management_handler::read_scale(const std::string &scale_ip)
{
    double ret = 0;
    device_config tmp;
    internal_get_device_config(tmp);
    for (auto &itr:tmp.scale)
    {
        if (itr.scale_ip == scale_ip)
        {
            ret = get_current_weight(scale_ip, ZH_SCALE_PORT, itr.scale_brand);
            ret *= itr.coefficient;
            break;
        }
    }

    return ret;
}

void system_management_handler::run_update(const std::string &ssid, const std::string &pack_path)
{
    auto user = zh_rpc_util_get_online_user(ssid, 0);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    int orig_fd = open(pack_path.c_str(), O_RDONLY);
    int new_fd = open("/root/install.sh", O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (orig_fd >= 0 && new_fd >= 0)
    {
        long buf;
        int read_len = 0;
        while ((read_len = read(orig_fd, &buf, sizeof(buf))) > 0)
        {
            write(new_fd, &buf, read_len);
        }
    }
    if (orig_fd >= 0)
    {
        close(orig_fd);
    }
    if (new_fd >= 0)
    {
        close(new_fd);
    }
    _exit(-1);
}

void system_management_handler::get_domain_name(std::string &_return)
{
    _return = std::string(getenv("BASE_URL")) + std::string(getenv("URL_REMOTE"));
}

void system_management_handler::get_oem_name(std::string &_return)
{
    _return = std::string(getenv("OEM_NAME"));
}

void system_management_handler::get_all_scale_brand(std::vector<std::string> &_return)
{
    _return = zh_scale_if::get_all_brand();
}

#define ZH_GET_DEVICE_HEALTH(_X) (_X.length() == 0?-1:m_get_device_health_map()[_X])

void system_management_handler::get_device_health(std::vector<device_health> &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 2);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    device_config dc;
    internal_get_device_config(dc);
    for (auto &itr : dc.gate)
    {
        device_health tmp;
        tmp.name = itr.name;
        tmp.entry_cam = ZH_GET_DEVICE_HEALTH(itr.entry_config.cam_ip);
        tmp.entry_led = ZH_GET_DEVICE_HEALTH(itr.entry_config.led_ip);
        tmp.entry_id = ZH_GET_DEVICE_HEALTH(itr.entry_id_reader_ip);
        tmp.entry_qr = ZH_GET_DEVICE_HEALTH(itr.entry_qr_ip);
        tmp.exit_cam = ZH_GET_DEVICE_HEALTH(itr.exit_config.cam_ip);
        tmp.exit_led = ZH_GET_DEVICE_HEALTH(itr.exit_config.led_ip);
        tmp.exit_id = ZH_GET_DEVICE_HEALTH(itr.exit_id_reader_ip);
        tmp.exit_qr = ZH_GET_DEVICE_HEALTH(itr.exit_qr_ip);
        _return.push_back(tmp);
    }
    for (auto &itr : dc.scale)
    {
        device_health tmp;
        tmp.name = itr.name;
        tmp.entry_cam = ZH_GET_DEVICE_HEALTH(itr.entry_config.cam_ip);
        tmp.entry_led = ZH_GET_DEVICE_HEALTH(itr.entry_config.led_ip);
        tmp.entry_id = ZH_GET_DEVICE_HEALTH(itr.entry_id_reader_ip);
        tmp.entry_qr = ZH_GET_DEVICE_HEALTH(itr.entry_qr_ip);
        tmp.entry_printer = ZH_GET_DEVICE_HEALTH(itr.entry_printer_ip);
        tmp.raster1 = ZH_GET_DEVICE_HEALTH(itr.raster_ip[0]);
        tmp.raster2 = ZH_GET_DEVICE_HEALTH(itr.raster_ip[1]);
        tmp.scale = ZH_GET_DEVICE_HEALTH(itr.scale_ip);
        tmp.exit_cam = ZH_GET_DEVICE_HEALTH(itr.exit_config.cam_ip);
        tmp.exit_led = ZH_GET_DEVICE_HEALTH(itr.exit_config.led_ip);
        tmp.exit_id = ZH_GET_DEVICE_HEALTH(itr.exit_id_reader_ip);
        tmp.exit_qr = ZH_GET_DEVICE_HEALTH(itr.exit_qr_ip);
        tmp.exit_printer = ZH_GET_DEVICE_HEALTH(itr.exit_printer_ip);
        _return.push_back(tmp);
    }
}

std::map<std::string, long> &system_management_handler::m_get_device_health_map()
{
    return zh_runtime_get_device_health();
}

void system_management_handler::read_qr(std::string &_return, const std::string &id_reader_ip)
{
    _return = qr_result[id_reader_ip];
}
bool system_management_handler::led_cast_welcome(const std::string &led_ip)
{
    return zh_hk_cast_welcome(led_ip, "蒙A12345");
}

void system_management_handler::trigger_cap(const std::string &ssid, const std::string &cam_ip)
{
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    zh_hk_manual_trigger(cam_ip);
}


static neb::CJsonObject get_cur_config_json()
{
    std::ifstream config_file("/conf/device/device_config.json", std::ios::in);
    std::istreambuf_iterator<char> beg(config_file), end;
    std::string config_string(beg, end);
    neb::CJsonObject config(config_string);

    return config;
}

bool system_management_handler::is_auto_confirm(const std::string &ssid)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto config = get_cur_config_json();
    config.Get("auto_confirm", ret);

    return ret;
}
void system_management_handler::set_auto_confirm(const std::string &ssid, const bool auto_set)
{
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto config = get_cur_config_json();
    config.Delete("auto_confirm");
    config.Add("auto_confirm", auto_set, auto_set);
    std::ofstream config_file("/conf/device/device_config.json", std::ios::out);
    config_file << config.ToFormattedString();
    config_file.close();
}

void system_management_handler::manual_confirm_scale(const std::string &ssid, const std::string &scale_name)
{
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto ssm = vehicle_order_center_handler::ssm_map[scale_name];
    if (ssm)
    {
        ssm->proc_manual_confirm_scale();
        ssm->trigger_sm();
    }
}

bool system_management_handler::upload_prompt_image(const std::string &ssid, const std::string &attachment)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    if (attachment.length() > 0)
    {
        zh_sql_prompt_image tmp;
        zh_sql_file att_file;
        att_file.save_file(attachment.substr(0, attachment.find_last_of('.')), std::to_string(time(nullptr)) + attachment.substr(attachment.find_last_of('.'), attachment.length()));
        att_file.insert_record();
        tmp.set_parent(att_file, "attachment");
        ret = tmp.insert_record();
    }

    return ret;
}
void system_management_handler::get_all_prompt_image(std::vector<prompt_image_info> &_return)
{
    auto all_img = sqlite_orm::search_record_all<zh_sql_prompt_image>();
    for (auto &itr:all_img)
    {
        auto att_file = itr.get_parent<zh_sql_file>("attachment");
        if (att_file)
        {
            prompt_image_info tmp;
            tmp.id = itr.get_pri_id();
            tmp.attachment_path = att_file->name;
            _return.push_back(tmp);
        }
    }
}
bool system_management_handler::delete_prompt_image(const std::string &ssid, const int64_t id)
{
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto prompt_img = sqlite_orm::search_record<zh_sql_prompt_image>(id);
    if (prompt_img)
    {
        prompt_img->remove_record();
    }

    return true;
}