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
public:
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
    virtual bool ctrl_led(const std::string &gate_code, const std::string &content);
    virtual bool ctrl_voice(const std::string &gate_code, const std::string &content);
    road_status get_status_by_road(const std::string &_road);
    void set_status_by_road(const std::string &_road, road_status &status);
    virtual void get_road_status(road_status &_return, const std::string &gate_code);
    void internal_get_device_config(device_config &_return);
    virtual double read_scale(const std::string &scale_ip);
    virtual void run_update(const std::string &ssid, const std::string &pack_path);
    virtual void get_domain_name(std::string &_return);
    virtual void get_oem_name(std::string &_return);
};

#endif // _SYSTEM_MANAGEMENT_IMP_H_
