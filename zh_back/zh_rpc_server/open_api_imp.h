#if !defined(_OPEN_API_IMP_H_)
#define _OPEN_API_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/open_api.h"
#include "../zh_tdf/tdf_include.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"
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
};

#endif // _OPEN_API_IMP_H_
