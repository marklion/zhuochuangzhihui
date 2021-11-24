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
        zh_hk_subcribe_event();
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
};

#endif // _OPEN_API_IMP_H_
