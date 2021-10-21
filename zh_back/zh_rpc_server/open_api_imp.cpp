#include "open_api_imp.h"
#include "system_management_imp.h"
#include "vehicle_order_center_imp.h"

#define OPEN_API_LOG_AUDIT(x) log_api_audit(__FUNCTION__, (x))

open_api_handler*open_api_handler::m_inst = nullptr;

void open_api_handler::log_api_audit(const char *_funcion_name, const std::string &_params)
{
    m_log.log("calling api function %s, params is %s", _funcion_name, _params.c_str());
}

bool open_api_handler::vehicle_come(const std::string &plateNo, const std::string &road)
{
    OPEN_API_LOG_AUDIT("road:" + road + ",plateNo:" + plateNo);
    auto status = system_management_handler::get_inst()->get_status_by_road(road);
    status.coming_vehicle = plateNo;
    system_management_handler::get_inst()->set_status_by_road(road, status);
    vehicle_state_machine::fetch_sm(plateNo, 1).proc_gate_near(road);

    return true;
}

bool open_api_handler::vehicle_leave(const std::string &plateNo, const std::string &road)
{
    OPEN_API_LOG_AUDIT("road:" + road + ",plateNo:" + plateNo);
    auto status = system_management_handler::get_inst()->get_status_by_road(road);
    status.coming_vehicle = "";
    system_management_handler::get_inst()->set_status_by_road(road, status);

    vehicle_state_machine::fetch_sm(plateNo, 1).proc_pass_gate(road);
    vehicle_state_machine::delete_sm(plateNo);

    return true;
}