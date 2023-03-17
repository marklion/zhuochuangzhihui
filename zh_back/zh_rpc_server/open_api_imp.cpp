#include "open_api_imp.h"
#include "system_management_imp.h"
#include "vehicle_order_center_imp.h"
#include "user_management_imp.h"
#include <sys/wait.h>

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

    return true;
}

bool open_api_handler::external_trigger_gate_vehicle(const std::string &road_ip, const std::string &plate_no)
{

    return false;
}
bool open_api_handler::external_trigger_gate_id(const std::string &id_reader_ip, const std::string &id_no, const std::string &road_ip)
{
    bool ret = false;

    return ret;

}
bool open_api_handler::external_trigger_scale_vehicle(const std::string &road_ip, const std::string &plate_no, const std::string &scale_name)
{
    bool ret = false;

    return ret;
}
bool open_api_handler::external_trigger_scale_id(const std::string &id_reader_ip, const std::string &id_no, const std::string &scale_name)
{
    bool ret = false;
    return ret;
}

bool open_api_handler::external_trigger_gate_qr(const std::string &road_ip, const std::string &qr_code)
{
    bool ret = false;


    return ret;
}
bool open_api_handler::external_trigger_scale_qr(const std::string &scale_ip, const std::string &qr_code, const std::string &scale_name)
{
    bool ret = false;

    return ret;
}

void open_api_handler::get_video(std::string &_return, const std::string &nvr_ip, const int64_t channel_id, const std::string &start_time, const std::string &stop_time)
{
}

void open_api_handler::get_all_video_path(std::vector<video_param> &_return)
{
    auto vps = sqlite_orm::search_record_all<zh_sql_video_path>();
    for (auto &itr:vps)
    {
        video_param tmp;
        tmp.id = itr.get_pri_id();
        tmp.name = itr.name;
        tmp.path = itr.path;
        _return.push_back(tmp);
    }
}

bool open_api_handler::add_video_path(const std::string &ssid, const video_param &_video_param)
{
    bool ret = false;

    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto exist_vp = sqlite_orm::search_record<zh_sql_video_path>("name == '%s'", _video_param.name.c_str());
    if (exist_vp)
    {
        exist_vp->path = _video_param.path;
        ret = exist_vp->update_record();
    }
    else
    {
        zh_sql_video_path tmp;
        tmp.name = _video_param.name;
        tmp.path = _video_param.path;
        ret = tmp.insert_record();
    }

    return ret;
}
void open_api_handler::del_video_path(const std::string &ssid, const int64_t id)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto exist_vp = sqlite_orm::search_record<zh_sql_video_path>(id);
    if (exist_vp)
    {
        exist_vp->remove_record();
    }
}

static int g_video_push_pid = -1;

bool open_api_handler::set_video_path(const std::string &ssid, const std::string &_video_path)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    if (g_video_push_pid>0)
    {
        kill(g_video_push_pid,9);
        int status = 0;
        waitpid(g_video_push_pid, &status, 0);
        g_video_push_pid = -1;
    }
    auto pid = fork();
    if (pid > 0)
    {
        g_video_push_pid = pid;
        ret = true;
    }
    else
    {
        //-f flv -vcodec h264 -vprofile baseline -acodec aac -ar 44100 -strict -2 -ac 1 -f flv -s 640*360 -q 10 "rtmp://localhost:1935/live/home"";
        char path_buff[256] = {0};
        strcpy(path_buff, _video_path.c_str());
        char *argv[30] = {0};
        argv[0] = "ffmpeg";
        argv[1] = "-re";
        argv[2] = "-rtsp_transport";
        argv[3] = "tcp";
        argv[4] = "-i";
        argv[5] = path_buff;
        argv[6] = "-f";
        argv[7] = "flv";
        argv[8] = "-vcodec";
        argv[9] = "h264";
        argv[10] = "-vprofile";
        argv[11] = "baseline";
        argv[12] = "-acodec";
        argv[13] = "aac";
        argv[14] = "-ar";
        argv[15] = "44100";
        argv[16] = "-strict";
        argv[17] = "-2";
        argv[18] = "-ac";
        argv[19] = "1";
        argv[20] = "-f";
        argv[21] = "flv";
        argv[22] = "-s";
        argv[23] = "640*360";
        argv[24] = "-q";
        argv[25] = "10";
        argv[26] = "rtmp://localhost:1935/live/home";
        execv("/usr/bin/ffmpeg", argv);
    }

    return ret;
}

static std::string pri_open_phone_login(const std::string &_phone)
{
    std::string ret;

    auto umh = user_management_handler::get_inst();
    auto user_info = sqlite_orm::search_record<zh_sql_user_info>("phone == '%s'", _phone.c_str());
    if (user_info)
    {
        ret = umh->pri_user_login(*user_info);
    }

    return ret;
}

void open_api_handler::stop_video(const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    if (g_video_push_pid > 0)
    {
        kill(g_video_push_pid, 9);
        int status = 0;
        waitpid(g_video_push_pid, &status, 0);
        g_video_push_pid = -1;
    }
}
void open_api_handler::get_device_status(std::vector<device_status> &_return, const std::string &phone)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }

    auto smh = system_management_handler::get_inst();
    std::vector<device_status_internel> tmp;
    smh->get_all_device(tmp);
    for (auto &itr : tmp)
    {
        device_status t_ds;
        t_ds.cur_weight = zh_double2string_reserve2(itr.cur_weight);
        t_ds.enter_gate_is_close = itr.enter_gate_is_close;
        t_ds.exit_gate_is_close = itr.exit_gate_is_close;
        t_ds.name = itr.name;
        t_ds.scale_status = itr.cur_status;
        t_ds.type_id = itr.is_scale ? 2 : 1;
        _return.push_back(t_ds);
    }
}

void open_api_handler::do_device_opt_gate_control(const std::string &phone, const std::string &name, const bool is_enter, const bool is_open)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto smh = system_management_handler::get_inst();
    smh->gate_control(name, is_enter, !is_open, ssid);
}
void open_api_handler::do_device_opt_confirm_scale(const std::string &phone, const std::string &name)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto smh = system_management_handler::get_inst();
    smh->confirm_weight(name, ssid);
}
void open_api_handler::do_device_opt_reset_scale(const std::string &phone, const std::string &name)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto smh = system_management_handler::get_inst();
    smh->reset_sm(name, ssid);
}
void open_api_handler::do_device_opt_trigger_cap(const std::string &phone, const std::string &name, const bool is_enter, const std::string &vehicle_number)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto smh = system_management_handler::get_inst();
    smh->manual_trigger(name, is_enter, vehicle_number, ssid);
}
void open_api_handler::do_device_opt_take_pic(std::string &_return, const std::string &phone, const std::string &name, const bool is_enter)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto smh = system_management_handler::get_inst();
    smh->take_picture(_return, name, is_enter, ssid);
}
void open_api_handler::get_queue_node(std::vector<field_queue_node> &_return, const std::string &phone)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto voch = vehicle_order_center_handler::get_inst();
    std::vector<vehicle_order_detail> all_node;
    voch->get_registered_vehicle(all_node, ssid);
    for (auto &itr : all_node)
    {
        field_queue_node tmp;
        tmp.back_plate = itr.basic_info.behind_vehicle_number;
        tmp.driver_name = itr.basic_info.driver_name;
        tmp.driver_phone = itr.basic_info.driver_phone;
        tmp.id = itr.basic_info.id;
        tmp.m_weight = zh_double2string_reserve2(itr.basic_info.m_weight);
        tmp.need_confirm = !itr.confirmed;
        tmp.p_weight = zh_double2string_reserve2(itr.basic_info.p_weight);
        tmp.plate = itr.basic_info.main_vehicle_number;
        tmp.seal_no = itr.basic_info.seal_no;
        tmp.status_code = itr.basic_info.status;
        tmp.stuff_name = itr.basic_info.stuff_name;
        tmp.call_time = itr.call_time;
        tmp.check_in_time = itr.checkin_time;
        tmp.has_called = itr.has_called;
        _return.push_back(tmp);
    }
}
bool open_api_handler::field_queue_call(const std::string &phone, const int64_t id, const bool is_call)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto voch = vehicle_order_center_handler::get_inst();
    return voch->call_vehicle(ssid, id, !is_call);
}
bool open_api_handler::field_queue_pass(const std::string &phone, const int64_t id)
{
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto voch = vehicle_order_center_handler::get_inst();
    return voch->driver_check_in(id, true, "", "");
}
bool open_api_handler::field_queue_confirm(const std::string &phone, const int64_t id, const bool is_confirm)
{
    bool ret = false;
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto voch = vehicle_order_center_handler::get_inst();
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(id);
    if (vo)
    {
        ret = voch->confirm_order_deliver(ssid, vo->order_number, is_confirm, "");
    }
    return ret;
}
bool open_api_handler::field_queue_set_seal(const std::string &phone, const int64_t id, const std::string &seal_no)
{
    bool ret = false;
    auto ssid = pri_open_phone_login(phone);
    if (ssid.length() <= 0)
    {
        ZH_RETURN_DUP_USER_MSG();
    }
    auto voch = vehicle_order_center_handler::get_inst();
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(id);
    if (vo)
    {
        ret = voch->set_seal_no(ssid, vo->order_number, seal_no);
    }
    return ret;
}
