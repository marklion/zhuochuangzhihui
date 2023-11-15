#include "device_management_imp.h"
#include "rpc_include.h"
#include <wait.h>

struct driver_run_status
{
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

static int start_daemon(const std::string &_cmd, const std::string &_args, unsigned short _port, int64_t device_id)
{
    int pid = 12;

    auto exec_args = util_split_string(_args, " ");
    exec_args.push_back("-p");
    exec_args.push_back(std::to_string(_port));
    exec_args.push_back("-i");
    exec_args.push_back(std::to_string(device_id));
    exec_args.insert(exec_args.begin(), _cmd);

    auto args_size = exec_args.size();

    pid = fork();
    if (pid <= 0)
    {
        char **argv = (char **)malloc(args_size * (sizeof(argv) + 1));
        for (size_t i = 0; i < args_size; i++)
        {
            argv[i] = (char *)malloc(exec_args[i].length() + 1);
            strcpy(argv[i], exec_args[i].c_str());
        }
        argv[args_size] = 0;
        execv(_cmd.c_str(), argv);
    }

    return pid;
}

static void stop_daemon(int64_t pid)
{
    kill(pid, 9);
    int ret_v = 0;
    waitpid(pid, &ret_v, 0);
}

device_management_handler::device_management_handler()
{
    pthread_mutexattr_t tmp;
    pthread_mutexattr_init(&tmp);
    pthread_mutexattr_settype(&tmp, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&map_lock, &tmp);
}

void device_management_handler::init_all_set()
{
    THR_DEF_CIENT(config_management);
    THR_CONNECT(config_management);
    std::vector<device_gate_set> gate_tmp;
    client->get_gate_config(gate_tmp);
    std::vector<device_scale_set> scale_tmp;
    client->get_scale_config(scale_tmp);
    TRH_CLOSE();

    for (auto &itr : gate_tmp)
    {
        start_device_no_exp(itr.gate.back.id);
        start_device_no_exp(itr.gate.front.id);
        start_device_no_exp(itr.id_reader.back.id);
        start_device_no_exp(itr.id_reader.front.id);
        start_device_no_exp(itr.led.back.id);
        start_device_no_exp(itr.led.front.id);
        start_device_no_exp(itr.plate_cam.back.id);
        start_device_no_exp(itr.plate_cam.front.id);
        start_device_no_exp(itr.qr_reader.back.id);
        start_device_no_exp(itr.qr_reader.front.id);
        start_device_no_exp(itr.speaker.back.id);
        start_device_no_exp(itr.speaker.front.id);
        start_device_no_exp(itr.video_cam.back.id);
        start_device_no_exp(itr.video_cam.front.id);
        sm_init_add(std::make_shared<gate_sm>(itr.id), itr.id);
    }
    for (auto &itr : scale_tmp)
    {
        start_device_no_exp(itr.gate.back.id);
        start_device_no_exp(itr.gate.front.id);
        start_device_no_exp(itr.id_reader.back.id);
        start_device_no_exp(itr.id_reader.front.id);
        start_device_no_exp(itr.led.back.id);
        start_device_no_exp(itr.led.front.id);
        start_device_no_exp(itr.plate_cam.back.id);
        start_device_no_exp(itr.plate_cam.front.id);
        start_device_no_exp(itr.qr_reader.back.id);
        start_device_no_exp(itr.qr_reader.front.id);
        start_device_no_exp(itr.speaker.back.id);
        start_device_no_exp(itr.speaker.front.id);
        start_device_no_exp(itr.video_cam.back.id);
        start_device_no_exp(itr.video_cam.front.id);
        start_device_no_exp(itr.printer.back.id);
        start_device_no_exp(itr.printer.front.id);
        start_device_no_exp(itr.scale.id);
    }
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
        ZH_RETURN_MSG(dm->name + "驱动不存在");
    }

    if (start)
    {
        if (get_status_from_map(device_id))
        {
            ZH_RETURN_MSG(dm->name + "早已启动");
        }
        driver_run_status tmp;
        tmp.port = g_base_port++;
        tmp.device_id = device_id;
        tmp.pid = start_daemon(dr->path, dm->args, tmp.port, device_id);
        if (tmp.pid >= 0)
        {
            add_status_to_map(tmp);
            ret = true;
        }
        else
        {
            ZH_RETURN_MSG(dm->name + "启动失败");
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
        client->cap_picture_slow(_return, cam_id);
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

void device_management_handler::push_scale_read(const int64_t scale_id, const double weight)
{
}

void device_management_handler::push_id_read(const int64_t id_id, const std::string &id_number)
{
}

void device_management_handler::push_qr_read(const int64_t qr_id, const std::string &qr_content)
{
}

void device_management_handler::push_plate_read(const int64_t plate_cam_id, const std::string &plate_no)
{
    auto pc_device = sqlite_orm::search_record<sql_device_meta>(plate_cam_id);
    if (pc_device)
    {
        auto set = pc_device->get_children<sql_device_set>("front_plate_cam");
        if (!set)
        {
            set.reset(pc_device->get_children<sql_device_set>("back_plate_cam").release());
        }
        if (set)
        {
            sm_trigger(
                set->get_pri_id(),
                [&](abs_state_machine &sm)
                {
                    bool ret = false;
                    auto &gsm = dynamic_cast<gate_sm &>(sm);
                    auto result = set->should_handle_income_plate(plate_no, gsm.order_number);
                    if (result.length() > 0)
                    {
                        led_display(get_same_side_device(plate_cam_id, "led"), {result});
                        speaker_cast(get_same_side_device(plate_cam_id, "speaker"), result);
                    }
                    else
                    {
                        ret = true;
                        gsm.trigger_device_id = plate_cam_id;
                        gsm.pass_plate_number = plate_no;
                    }

                    return ret;
                });
        }
    }
}

void device_management_handler::start_device_no_exp(int64_t id)
{
    try
    {
        device_ctrl(id, true);
    }
    catch (gen_exp e)
    {
    }
}

void device_management_handler::sm_init_add(std::shared_ptr<abs_state_machine> _sm, int64_t sm_id)
{
    pthread_mutex_lock(&map_lock);
    m_sm_map[sm_id] = _sm;
    _sm->init_sm();
    pthread_mutex_unlock(&map_lock);
}

void device_management_handler::sm_trigger(int64_t sm_id, std::function<bool(abs_state_machine &_sm)> update_func)
{
    pthread_mutex_lock(&map_lock);
    auto sm = m_sm_map[sm_id];
    if (sm && update_func(*sm))
    {
        sm->trigger_sm();
    }
    pthread_mutex_unlock(&map_lock);
}

#define GOTHROUGH_EACH_SET(x, y)                                 \
    if (x)                                                       \
    {                                                            \
        auto target = x->get_parent<sql_device_meta>(y + _type); \
        if (target)                                              \
        {                                                        \
            ret = target->get_pri_id();                          \
        }                                                        \
    }

int64_t device_management_handler::get_same_side_device(int64_t _input_id, const std::string &_type)
{
    int64_t ret = 0;

    std::string front_direc = "front_";
    std::string back_direc = "back_";
    auto ipd = sqlite_orm::search_record<sql_device_meta>(_input_id);
    if (ipd)
    {
        auto fpc_set = ipd->get_children<sql_device_set>("front_plate_cam");
        auto bpc_set = ipd->get_children<sql_device_set>("back_plate_cam");
        auto fi_set = ipd->get_children<sql_device_set>("front_id_reader");
        auto bi_set = ipd->get_children<sql_device_set>("back_id_reader");
        auto fq_set = ipd->get_children<sql_device_set>("front_qr_reader");
        auto bq_set = ipd->get_children<sql_device_set>("back_qr_reader");
        GOTHROUGH_EACH_SET(fpc_set, front_direc);
        GOTHROUGH_EACH_SET(bpc_set, back_direc);
        GOTHROUGH_EACH_SET(fi_set, front_direc);
        GOTHROUGH_EACH_SET(bi_set, back_direc);
        GOTHROUGH_EACH_SET(fq_set, front_direc);
        GOTHROUGH_EACH_SET(bq_set, back_direc);
    }

    return ret;
}

gate_sm::gate_sm(int64_t _set_id) : abs_state_machine(std::unique_ptr<abs_sm_state>(new gate_state_init())), set_id(_set_id)
{
}

void gate_state_init::before_enter(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<gate_sm &>(_sm);
    auto this_gate = get_gate_config_by_id(sm.set_id);
    if (this_gate)
    {
        THR_CALL_DM_BEGIN();
        client->led_display(this_gate->led.back.id, {"欢迎光临"});
        client->led_display(this_gate->led.front.id, {"欢迎光临"});
        THR_CALL_DM_END();
    }
    sm.order_number.clear();
    sm.pass_plate_number.clear();
    sm.trigger_device_id = 0;
}

void gate_state_init::after_exit(abs_state_machine &_sm)
{
}

std::unique_ptr<abs_sm_state> gate_state_init::proc_event(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<gate_sm &>(_sm);
    auto this_gate = get_gate_config_by_id(sm.set_id);
    if (this_gate)
    {
        THR_CALL_DM_BEGIN();
        auto gate_id = device_management_handler::get_same_side_device(sm.trigger_device_id, "gate");
        auto speaker_id = device_management_handler::get_same_side_device(sm.trigger_device_id, "speaker");
        auto led_id = device_management_handler::get_same_side_device(sm.trigger_device_id, "led");
        client->gate_ctrl(gate_id, true);
        client->speaker_cast(speaker_id, "aaac");
        client->led_display(led_id, {"asdfasdf", sm.pass_plate_number});
        THR_CALL_DM_END();
        THR_CALL_BEGIN(order_center);
        client->order_push_gate(sm.order_number, "自动");
        THR_CALL_END();
    }
    sm.init_sm();

    return std::unique_ptr<abs_sm_state>();
}

std::unique_ptr<device_gate_set> get_gate_config_by_id(int64_t _id)
{
    std::unique_ptr<device_gate_set> ret;
    std::vector<device_gate_set> gate_tmp;
    try
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        client->get_gate_config(gate_tmp);
        TRH_CLOSE(); /* code */
    }
    catch (...)
    {
    }

    for (auto &itr : gate_tmp)
    {
        if (itr.id == _id)
        {
            ret.reset(new device_gate_set(itr));
            break;
        }
    }

    return ret;
}

std::unique_ptr<device_scale_set> get_scale_config_by_id(int64_t _id)
{
    std::unique_ptr<device_scale_set> ret;
    std::vector<device_scale_set> scale_tmp;
    try
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        client->get_scale_config(scale_tmp);
        TRH_CLOSE(); /* code */
    }
    catch (...)
    {
    }

    for (auto &itr : scale_tmp)
    {
        if (itr.id == _id)
        {
            ret.reset(new device_scale_set(itr));
            break;
        }
    }
    return ret;
}
