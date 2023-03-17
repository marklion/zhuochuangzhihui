#include "system_management_imp.h"
#include "../zh_database/zh_db_config.h"
#include "zh_rpc_util.h"
#include <iostream>
#include <iosfwd>
#include <fstream>
#include "../zh_raster/lib/zh_raster.h"
#include "../zh_id_reader/lib/zh_id_reader.h"
#include "../zh_scale/lib/zh_scale.h"
#include "vehicle_order_center_imp.h"
#include <fstream>
#include "../local_dev/state_machine/lib/state_machine_control.h"

system_management_handler *system_management_handler::m_inst = nullptr;

bool system_management_handler::reboot_system(const std::string &ssid)
{
    if (!zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false))
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    _exit(-1);
}
void system_management_handler::current_version(std::string &_return)
{
    std::ifstream version_file("/conf/version.txt");
    std::getline(version_file, _return);
}

void system_management_handler::run_update(const std::string &ssid, const std::string &pack_path)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto config = get_cur_config_json();
    config.Delete("auto_confirm");
    config.Add("auto_confirm", auto_set, auto_set);
    std::ofstream config_file("/conf/device/device_config.json", std::ios::out);
    config_file << config.ToFormattedString();
    config_file.close();
}

bool system_management_handler::upload_prompt_image(const std::string &ssid, const std::string &attachment)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto prompt_img = sqlite_orm::search_record<zh_sql_prompt_image>(id);
    if (prompt_img)
    {
        prompt_img->remove_record();
    }

    return true;
}

void system_management_handler::get_company_address_info(company_address_info &_return)
{
    auto config = get_cur_config_json();
    if (config.KeyExist("company_address_info"))
    {
        auto cai = config["company_address_info"];
        cai.Get("lat", _return.x);
        cai.Get("lag", _return.y);
        cai.Get("distance", _return.distance);
    }

}
bool system_management_handler::set_company_address_info(const std::string &ssid, const company_address_info &address_info)
{
    bool ret = false;

    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto config = get_cur_config_json();
    auto new_config = neb::CJsonObject();
    new_config.Add("lat", address_info.x);
    new_config.Add("lag", address_info.y);
    new_config.Add("distance", address_info.distance);

    config.Delete("company_address_info");
    config.Add("company_address_info", new_config);

    std::ofstream config_file("/conf/device/device_config.json", std::ios::out);
    config_file << config.ToFormattedString();
    config_file.close();
    ret = true;

    return ret;
}

void system_management_handler::get_register_info(register_config_info &_return)
{
    auto config = get_cur_config_json();
    if (config.KeyExist("register_config"))
    {
        config["register_config"].Get("enable_register", _return.enabled);
        config["register_config"].Get("pass_time", _return.pass_time);
        config["register_config"].Get("check_in_time", _return.check_in_time);
        config["register_config"].Get("leave_limit", _return.leave_limit);
    }
    else
    {
        _return.enabled = false;
    }
}
bool system_management_handler::set_register_info(const std::string &ssid, const register_config_info &register_config)
{
    bool ret = false;

    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto config = get_cur_config_json();
    auto new_config = neb::CJsonObject();
    new_config.Add("enable_register", register_config.enabled, register_config.enabled);
    new_config.Add("pass_time", register_config.pass_time);
    new_config.Add("check_in_time", register_config.check_in_time);
    new_config.Add("leave_limit", register_config.leave_limit);

    config.Delete("register_config");
    config.Add("register_config", new_config);

    std::ofstream config_file("/conf/device/device_config.json", std::ios::out);
    config_file << config.ToFormattedString();
    config_file.close();
    ret = true;
    return ret;
}




static void pri_change_device_state(const std::string &_device_name)
{
    std::ifstream config_file_orig("/conf/device/device_config.json", std::ios::in);
    std::istreambuf_iterator<char> beg(config_file_orig), end;
    std::string config_string(beg, end);
    neb::CJsonObject tmp(config_string);
    auto &scale_configs = tmp["scale"];
    auto &gate_configs = tmp["gate"];
    for (auto i = 0; i < scale_configs.GetArraySize(); i++)
    {
        if (scale_configs[i]("name") == _device_name)
        {
            bool orig_pause_state = false;
            if (scale_configs[i].Get("pause", orig_pause_state) && orig_pause_state == true)
            {
                scale_configs[i].Delete("pause");
                scale_configs[i].Add("pause", false, false);
            }
            else
            {
                scale_configs[i].Delete("pause");
                scale_configs[i].Add("pause", true, true);
            }
            break;
        }
    }
    for (auto i = 0; i < gate_configs.GetArraySize(); i++)
    {
        if (gate_configs[i]("name") == _device_name)
        {
            bool orig_pause_state = false;
            if (gate_configs[i].Get("pause", orig_pause_state) && orig_pause_state == true)
            {
                gate_configs[i].Delete("pause");
                gate_configs[i].Add("pause", false, false);
            }
            else
            {
                gate_configs[i].Delete("pause");
                gate_configs[i].Add("pause", true, true);
            }
            break;
        }
    }
    std::ofstream config_file("/conf/device/device_config.json", std::ios::out);
    config_file << tmp.ToFormattedString();
    config_file.close();
}

bool system_management_handler::need_seal_no()
{
    bool ret = false;

    auto config = get_cur_config_json();
    if (config("need_seal_no") == "true")
    {
        ret = true;
    }

    return ret;
}
bool system_management_handler::set_need_seal_no(const std::string &ssid, const bool need_sn)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER);

    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto config = get_cur_config_json();
    config.Delete("need_seal_no");
    config.Add("need_seal_no", need_sn, need_sn);
    std::ofstream config_file("/conf/device/device_config.json", std::ios::out);
    config_file << config.ToFormattedString();
    config_file.close();

    return true;
}

bool system_management_handler::check_in_enabled()
{
    bool ret = true;

    auto config = get_cur_config_json();
    if (config("disable_check_in") == "true")
    {
        ret = false;
    }

    return ret;
}
void system_management_handler::get_all_device(std::vector<device_status_internel> &_return)
{
    auto all_device = sm_control_get_all_sm();
    for (auto &itr:all_device)
    {
        device_status_internel tmp;
        tmp.cur_status = itr.status_string;
        tmp.cur_weight = itr.cur_weight;
        tmp.enter_gate_is_close = itr.enter_gate_is_close;
        tmp.exit_gate_is_close = itr.exit_gate_is_close;
        tmp.is_scale = itr.sm_type=="scale"?true:false;
        tmp.name = itr.name;
        _return.push_back(tmp);
    }
}
void system_management_handler::gate_control(const std::string &name, const bool is_enter, const bool is_close, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    sm_control_open_gate(name, is_enter, !is_close);
}
void system_management_handler::take_picture(std::string &_return, const std::string &name, const bool is_enter, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    _return = sm_control_take_picture(name, is_enter);
}
void system_management_handler::get_video(std::string &_return, const std::string &name, const bool is_enter, const std::string &begin_date, const std::string &end_date, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    _return = sm_control_get_record(name, is_enter, begin_date, end_date);
}
void system_management_handler::manual_trigger(const std::string &name, const bool is_enter, const std::string &plate, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    sm_control_trigger(name, is_enter, plate);
}
void system_management_handler::reset_sm(const std::string &name, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    sm_control_reset(name);
}
void system_management_handler::confirm_weight(const std::string &name, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    sm_control_confirm_weight(name);
}
void system_management_handler::led_cast(const std::string &name, const bool is_enter, const std::string &content, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    sm_control_led_cast(name, is_enter, content);
}
void system_management_handler::printer_print(const std::string &name, const bool is_enter, const std::string &content, const std::string &qr_content, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    sm_control_print(name, is_enter, content, qr_content);
}

void system_management_handler::set_traffic_light(const std::string& name, const bool is_enter, const bool is_green, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    sm_control_set_traffic_light(name, is_enter, is_green);
}

void system_management_handler::get_core_config(std::string& _return, const std::string& ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto pfp = popen("cat /database/core_config.yaml", "r");
    if (pfp)
    {
        char a;
        while (fread(&a, 1, 1, pfp) == 1)
        {
            _return.push_back(a);
        }
        pclose(pfp);
    }

}
void system_management_handler::set_core_config(const std::string &ssid, const std::string &config_content)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto pfp = popen("cat > /database/core_config.yaml", "w");
    if (pfp)
    {
        fwrite(config_content.data(), 1, config_content.length(), pfp);
        pclose(pfp);
    }
}