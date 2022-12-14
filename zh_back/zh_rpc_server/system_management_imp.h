#if !defined(_SYSTEM_MANAGEMENT_IMP_H_)
#define _SYSTEM_MANAGEMENT_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/system_management.h"
#include <map>
#include <pthread.h>

class system_management_handler : virtual public system_managementIf {
private:
    static system_management_handler *m_inst;
    std::map<std::string,road_status> m_road_status_map;
    pthread_mutex_t m_road_status_map_lock;
    system_management_handler() {
        pthread_mutex_init(&m_road_status_map_lock, NULL);
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
    virtual void get_device_config(device_config &_return, const std::string &ssid);
    virtual bool edit_device_config(const std::string &ssid, const device_config &config);
    virtual bool raster_is_block(const std::string &raster_ip);
    virtual bool print_content(const std::string &printer_ip, const std::string &content, const std::string &qr_code);
    virtual void read_id_no(std::string &_return, const std::string &id_reader_ip);
    virtual bool ctrl_gate(const std::string &gate_code, const int64_t cmd);
    road_status get_status_by_road(const std::string &_road);
    void set_status_by_road(const std::string &_road, road_status &status);
    virtual void get_road_status(road_status &_return, const std::string &gate_code);
    void internal_get_device_config(device_config &_return);
    virtual double read_scale(const std::string &scale_ip);
    virtual void run_update(const std::string &ssid, const std::string &pack_path);
    virtual void get_domain_name(std::string &_return);
    virtual void get_oem_name(std::string &_return);
    virtual void get_all_scale_brand(std::vector<std::string> &_return);
    virtual void get_device_health(std::vector<device_health> &_return, const std::string &ssid);
    virtual void read_qr(std::string &_return, const std::string &id_reader_ip);
    virtual bool led_cast_welcome(const std::string &led_ip);
    virtual void trigger_cap(const std::string &ssid, const std::string &cam_ip);
    virtual bool is_auto_confirm(const std::string &ssid);
    virtual void set_auto_confirm(const std::string &ssid, const bool auto_set);
    virtual void manual_confirm_scale(const std::string &ssid, const std::string &scale_name);
    virtual bool upload_prompt_image(const std::string &ssid, const std::string &attachment);
    virtual void get_all_prompt_image(std::vector<prompt_image_info> &_return);
    virtual bool delete_prompt_image(const std::string &ssid, const int64_t id);
    virtual void get_company_address_info(company_address_info &_return);
    virtual bool set_company_address_info(const std::string &ssid, const company_address_info &address_info);
    virtual void get_register_info(register_config_info &_return);
    virtual bool set_register_info(const std::string &ssid, const register_config_info &register_config);
    virtual void get_scale_state(std::vector<scale_state_info> &_return, const std::string &ssid);
    virtual void reset_scale_state(const std::string &ssid, const std::string &scale_name);
    virtual bool read_cam_io(const std::string &cam_ip);
    virtual bool switch_device_state(const std::string &ssid, const std::string &device_name);
    virtual bool need_seal_no();
    virtual bool set_need_seal_no(const std::string &ssid, const bool need_sn);
    virtual void trigger_cam_vehicle_number(const std::string &ssid, const std::string &vehicle_number, const std::string &device_ip, const std::string &device_name);
    virtual void get_cam_pic(std::string& _return, const std::string& ssid, const std::string& device_ip);
    bool check_in_enabled();
};

#endif // _SYSTEM_MANAGEMENT_IMP_H_
