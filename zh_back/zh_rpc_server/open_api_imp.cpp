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
    return true;
}

bool open_api_handler::vehicle_leave(const std::string &plateNo, const std::string &road)
{
    OPEN_API_LOG_AUDIT("road:" + road + ",plateNo:" + plateNo);
    auto status = system_management_handler::get_inst()->get_status_by_road(road);
    status.coming_vehicle = "";
    system_management_handler::get_inst()->set_status_by_road(road, status);

    if (sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100 AND status != 0", plateNo.c_str()))
    {
    }

    return true;
}

bool open_api_handler::external_trigger_gate_vehicle(const std::string &road_ip, const std::string &plate_no)
{
    bool ret = false;

    auto gsm = vehicle_order_center_handler::get_inst()->get_gate_sm(road_ip);
    if (gsm)
    {
        gsm->proc_trigger_vehicle_number(plate_no);
        gsm->trigger_sm();
    }

    return ret;
}
bool open_api_handler::external_trigger_gate_id(const std::string &id_reader_ip, const std::string &id_no, const std::string &road_ip)
{
    bool ret = false;
    auto gsm = vehicle_order_center_handler::get_inst()->get_gate_sm(road_ip);
    if (gsm)
    {
        gsm->proc_trigger_id_no(id_no);
        gsm->trigger_sm();
    }

    return ret;

}
bool open_api_handler::external_trigger_scale_vehicle(const std::string &road_ip, const std::string &plate_no, const std::string &scale_name)
{
    bool ret = false;
    auto ssm = vehicle_order_center_handler::get_inst()->get_scale_sm(scale_name);
    if (ssm)
    {
        ssm->proc_trigger_vehicle(plate_no, road_ip);
        ssm->trigger_sm();
    }

    return ret;
}
bool open_api_handler::external_trigger_scale_id(const std::string &id_reader_ip, const std::string &id_no, const std::string &scale_name)
{
    bool ret = false;
    auto ssm = vehicle_order_center_handler::get_inst()->get_scale_sm(scale_name);
    if (ssm)
    {
        ssm->proc_trigger_id_read(id_no, id_reader_ip);
        ssm->trigger_sm();
    }

    return ret;

}

bool open_api_handler::external_trigger_gate_qr(const std::string &road_ip, const std::string &qr_code)
{
    bool ret = false;
    auto gsm = vehicle_order_center_handler::get_inst()->get_gate_sm(road_ip);
    if (gsm)
    {
        gsm->proc_trigger_qr_code(qr_code + "\r");
        gsm->trigger_sm();
    }

    return ret;
}
bool open_api_handler::external_trigger_scale_qr(const std::string &scale_ip, const std::string &qr_code, const std::string &scale_name)
{
    bool ret = false;
    auto ssm = vehicle_order_center_handler::get_inst()->get_scale_sm(scale_name);
    if (ssm)
    {
        ssm->proc_trigger_qr(qr_code, scale_ip + "\r");
        ssm->trigger_sm();
    }

    return ret;
}
static NET_DVR_TIME strTime2unix(const std::string &timeStamp)
{
    NET_DVR_TIME tm;
    memset(&tm, 0, sizeof(tm));

    sscanf(
        timeStamp.c_str(), "%d-%d-%d %d:%d:%d",
        &tm.dwYear, &tm.dwMonth, &tm.dwDay,
        &tm.dwHour, &tm.dwMinute, &tm.dwSecond);

    return tm;
}
void open_api_handler::get_video(std::string &_return, const std::string &nvr_ip, const int64_t channel_id, const std::string &start_time, const std::string &stop_time)
{
    NET_DVR_TIME start = {0};
    NET_DVR_TIME end = {0};

    auto start_date_val = strTime2unix(start_time);
    auto end_date_val = strTime2unix(stop_time);

    _return = zh_hk_get_channel_video(nvr_ip, channel_id, start_date_val, end_date_val);
}