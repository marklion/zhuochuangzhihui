#if !defined(_OPEN_API_IMP_H_)
#define _OPEN_API_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/open_api.h"
#include "../zh_tdf/tdf_include.h"
#include "zh_rpc_util.h"
class open_api_handler:public open_apiIf {
private:
    static open_api_handler *m_inst;
    tdf_log m_log;
    open_api_handler():m_log("open_api") {
    }
public:
    static open_api_handler *get_inst() {
        if (m_inst == nullptr)
        {
            m_inst = new open_api_handler();
        }
        return m_inst;
    }
    void log_api_audit(const char *_funcion_name, const std::string &_params);
    virtual bool vehicle_come(const std::string &plateNo, const std::string &road);
    virtual bool vehicle_leave(const std::string &plateNo, const std::string &road);
    virtual bool external_trigger_gate_vehicle(const std::string &road_ip, const std::string &plate_no);
    virtual bool external_trigger_gate_id(const std::string &id_reader_ip, const std::string &id_no, const std::string &road_ip);
    virtual bool external_trigger_scale_vehicle(const std::string &road_ip, const std::string &plate_no, const std::string &scale_name);
    virtual bool external_trigger_scale_id(const std::string &id_reader_ip, const std::string &id_no, const std::string &scale_name);
    virtual bool external_trigger_gate_qr(const std::string &road_ip, const std::string &qr_code);
    virtual bool external_trigger_scale_qr(const std::string &scale_ip, const std::string &qr_code, const std::string &scale_name);
    virtual void get_video(std::string &_return, const std::string &nvr_ip, const int64_t channel_id, const std::string &start_time, const std::string &stop_time);
    virtual void get_all_video_path(std::vector<video_param> &_return);
    virtual bool add_video_path(const std::string &ssid, const video_param &_video_param);
    virtual void del_video_path(const std::string &ssid, const int64_t id);
    virtual bool set_video_path(const std::string &ssid, const std::string &_video_path);
    virtual void stop_video(const std::string& ssid);
};

#endif // _OPEN_API_IMP_H_
