#include "device_management_imp.h"
#include "rpc_include.h"

struct driver_run_status {
    int64_t device_id = 0;
    int pid = 0;
    unsigned short port = 0;
};

static pthread_mutex_t g_runing_lock = PTHREAD_MUTEX_INITIALIZER;
static std::map<ino64_t, driver_run_status> g_runing_map;
static unsigned short g_base_port = 31300;

static void add_status_to_map(const driver_run_status &_drs)
{
    pthread_mutex_lock(&g_runing_lock);
    g_runing_map[_drs.device_id] = _drs;
    pthread_mutex_unlock(&g_runing_lock);
}

static std::unique_ptr<driver_run_status> get_status_from_map(int64_t _device_id)
{
    std::unique_ptr<driver_run_status> ret;
    pthread_mutex_lock(&g_runing_lock);
    if (g_runing_map.find(_device_id) != g_runing_map.end())
    {
        ret.reset(new driver_run_status(g_runing_map[_device_id]));
    }
    pthread_mutex_unlock(&g_runing_lock);

    return ret;
}

static void del_status_from_map(int64_t _device_id)
{
    pthread_mutex_lock(&g_runing_lock);
    if (g_runing_map.find(_device_id) != g_runing_map.end())
    {
        g_runing_map.erase(_device_id);
    }
    pthread_mutex_unlock(&g_runing_lock);
}

static int start_daemon(const std::string &_cmd, const std::string &_args, unsigned short _port)
{
    int pid = 12;

    return pid;
}

static void stop_daemon(int64_t device_id)
{
}

bool device_management_handler::device_ctrl(const int64_t device_id, const bool start)
{
    bool ret = false;

    auto dm = sqlite_orm::search_record<sql_device_meta>(device_id);
    if (!dm)
    {
        ZH_RETURN_MSG("设备不存在");
    }
    auto dr = dm->get_parent<sql_device_driver>("driver");
    if (!dr)
    {
        ZH_RETURN_MSG("驱动不存在");
    }

    if (start)
    {
        if (get_status_from_map(device_id))
        {
            ZH_RETURN_MSG("早已启动");
        }
        driver_run_status tmp;
        tmp.port = g_base_port;
        tmp.device_id = device_id;
        tmp.pid = start_daemon(dr->path, dm->args, tmp.port);
        if (tmp.pid >= 0)
        {
            add_status_to_map(tmp);
            ret = true;
        }
        else
        {
            ZH_RETURN_MSG("启动失败");
        }
    }
    else
    {
        auto sp = get_status_from_map(device_id);
        if (sp)
        {
            stop_daemon(sp->pid);
            del_status_from_map(device_id);
        }
        ret = true;
    }

    return ret;
}

bool device_management_handler::device_is_started(const int64_t device_id)
{
    bool ret = false;
    if (get_status_from_map(device_id))
    {
        ret = true;
    }
    return ret;
}

void device_management_handler::gate_ctrl(const int64_t gate_id, const bool is_open)
{
    auto sp = get_status_from_map(gate_id);

    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        client->gate_ctrl(0, is_open);
        TRH_CLOSE();
    }
}

void device_management_handler::led_display(const int64_t led_id, const std::vector<std::string> &content)
{
    auto sp = get_status_from_map(led_id);

    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        client->led_display(0, content);
        TRH_CLOSE();
    }
}

void device_management_handler::speaker_cast(const int64_t speaker_id, const std::string &content)
{
    auto sp = get_status_from_map(speaker_id);

    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        client->speaker_cast(0, content);
        TRH_CLOSE();
    }
}

double device_management_handler::last_scale_read(const int64_t scale_id)
{
    double ret = 0;

    auto sp = get_status_from_map(scale_id);
    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        ret = client->last_scale_read(0);
        TRH_CLOSE();
    }

    return ret;
}

void device_management_handler::last_id_read(std::string &_return, const int64_t id_reader_id)
{
    auto sp = get_status_from_map(id_reader_id);
    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        client->last_id_read(_return, 0);
        TRH_CLOSE();
    }
}

void device_management_handler::last_qr_read(std::string &_return, const int64_t qr_reader_id)
{
    auto sp = get_status_from_map(qr_reader_id);
    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        client->last_qr_read(_return, 0);
        TRH_CLOSE();
    }
}

void device_management_handler::last_plate_read(std::string &_return, const int64_t plate_cam_id)
{
    auto sp = get_status_from_map(plate_cam_id);
    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        client->last_plate_read(_return, 0);
        TRH_CLOSE();
    }
}

void device_management_handler::cap_picture_slow(std::string &_return, const int64_t cam_id)
{
    auto sp = get_status_from_map(cam_id);
    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        client->cap_picture_slow(_return,  cam_id);
        TRH_CLOSE();
    }
}

void device_management_handler::video_record_slow(std::string &_return, const int64_t cam_id, const std::string &begin_date, const std::string &end_date)
{
    auto sp = get_status_from_map(cam_id);
    if (sp)
    {
        THR_DEF_CIENT(device_management);
        THR_CONNECT_DEV(device_management, sp->port);
        client->video_record_slow(_return, 0, begin_date, end_date);
        TRH_CLOSE();
    }
}
