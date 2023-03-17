#if !defined(_SYSTEM_MANAGEMENT_IMP_H_)
#define _SYSTEM_MANAGEMENT_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/system_management.h"
#include <map>
#include <pthread.h>

class system_management_handler : virtual public system_managementIf {
private:
    static system_management_handler *m_inst;
    system_management_handler() {
    }
    std::map<std::string, long> &m_get_device_health_map();
public:
    std::map<std::string, std::string> id_result;
    std::map<std::string, std::string> qr_result;
    std::map<std::string, std::string> cam_result;
    static system_management_handler *get_inst() {
        if (m_inst == nullptr)
        {
            m_inst = new system_management_handler();
        }
        return m_inst;
    }
    virtual bool reboot_system(const std::string &ssid);
    virtual void current_version(std::string &_return);
    virtual void run_update(const std::string &ssid, const std::string &pack_path);
    virtual void get_domain_name(std::string &_return);
    virtual void get_oem_name(std::string &_return);
    virtual bool is_auto_confirm(const std::string &ssid);
    virtual void set_auto_confirm(const std::string &ssid, const bool auto_set);
    virtual bool upload_prompt_image(const std::string &ssid, const std::string &attachment);
    virtual void get_all_prompt_image(std::vector<prompt_image_info> &_return);
    virtual bool delete_prompt_image(const std::string &ssid, const int64_t id);
    virtual void get_company_address_info(company_address_info &_return);
    virtual bool set_company_address_info(const std::string &ssid, const company_address_info &address_info);
    virtual void get_register_info(register_config_info &_return);
    virtual bool set_register_info(const std::string &ssid, const register_config_info &register_config);
    virtual bool need_seal_no();
    virtual bool set_need_seal_no(const std::string &ssid, const bool need_sn);
    bool check_in_enabled();
    virtual void get_all_device(std::vector<device_status_internel> &_return);
    virtual void gate_control(const std::string &name, const bool is_enter, const bool is_close, const std::string& ssid);
    virtual void take_picture(std::string &_return, const std::string &name, const bool is_enter, const std::string& ssid);
    virtual void get_video(std::string &_return, const std::string &name, const bool is_enter, const std::string &begin_date, const std::string &end_date, const std::string& ssid);
    virtual void manual_trigger(const std::string &name, const bool is_enter, const std::string &plate, const std::string& ssid);
    virtual void reset_sm(const std::string &name, const std::string& ssid);
    virtual void confirm_weight(const std::string &name, const std::string& ssid);
    virtual void led_cast(const std::string &name, const bool is_enter, const std::string &content, const std::string& ssid);
    virtual void printer_print(const std::string &name, const bool is_enter, const std::string &content, const std::string &qr_content, const std::string& ssid);
    virtual void set_traffic_light(const std::string& name, const bool is_enter, const bool is_green, const std::string& ssid);
    virtual void get_core_config(std::string& _return, const std::string& ssid);
    virtual void set_core_config(const std::string& ssid, const std::string& config_content);
};

#endif // _SYSTEM_MANAGEMENT_IMP_H_
