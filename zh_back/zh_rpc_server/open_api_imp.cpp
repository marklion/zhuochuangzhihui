#include "open_api_imp.h"
#include "system_management_imp.h"
#include "vehicle_order_center_imp.h"
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

void open_api_handler::stop_video(const std::string& ssid)
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