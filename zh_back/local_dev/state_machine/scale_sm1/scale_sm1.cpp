#include "../state_machine_common.h"
#include "scale_sm1.h"

class scale_sm1 : public abs_scale_state_machine, public epoll_node_t
{
public:
    using abs_scale_state_machine::abs_scale_state_machine;
    void led_show_and_cast(ssm_device_type _device_type, const std::string &_content, const std::string &_vehicle_number)
    {
        auto speaker_dev = enter_speaker;
        if (exit_led == _device_type)
        {
            speaker_dev = exit_speaker;
        }
        send_led_show_msg(_device_type, getenv("OEM_SHORT"), zh_rpc_util_get_timestring(), _content, _vehicle_number);
        send_speaker_speak_msg(speaker_dev, _content);
        send_speaker_speak_msg(speaker_dev, _vehicle_number);
    }
    void led_show(ssm_device_type _device_type, const std::string &_content1, const std::string &_content2)
    {
        send_led_show_msg(_device_type, getenv("OEM_SHORT"), zh_rpc_util_get_timestring(), _content1, _content2);
    }
    virtual bool proc_in()
    {
        proc_msg();
        sm_change();
        return true;
    }
    virtual int get_fd() const
    {
        return self_que_fd;
    }
};



static cur_enter_vehicle_type vehicle_is_in_system(const std::string &_vehicle_number)
{
    cur_enter_vehicle_type ret = enter_type_no_thing;

    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number = '%s' AND status != 100", _vehicle_number.c_str());
    if (vo)
    {
        ret = enter_type_vehicle_order;
    }
    else
    {
        auto white_record = sqlite_orm::search_record<zh_sql_vehicle>("main_vehicle_number = '%s' AND in_white_list == 1 AND use_stuff != ''", _vehicle_number.c_str());
        if (white_record)
        {
            ret = enter_type_white_record;
        }
    }

    return ret;
}

static std::unique_ptr<focus_face_set> get_cur_face(ssm_device_type &_device_type)
{
    std::unique_ptr<focus_face_set> ret;
    bool same_face = false;

    switch (_device_type)
    {
    case enter_cam:
    case enter_id:
    case enter_qr:
    {
        same_face = true;
        ret.reset(new focus_face_set());
        break;
    }
    case exit_cam:
    case exit_id:
    case exit_qr:
    {
        same_face = false;
        ret.reset(new focus_face_set());
        break;
    }

    default:
        break;
    }
    if (ret)
    {
        if (same_face)
        {
            ret->cur_enter_cam = enter_cam;
            ret->cur_enter_gate = enter_gate;
            ret->cur_enter_led = enter_led;
            ret->cur_exit_cam = exit_cam;
            ret->cur_exit_gate = exit_gate;
            ret->cur_exit_led = exit_led;
        }
        else
        {
            ret->cur_enter_cam = exit_cam;
            ret->cur_enter_gate = exit_gate;
            ret->cur_enter_led = exit_led;
            ret->cur_exit_cam = enter_cam;
            ret->cur_exit_gate = enter_gate;
            ret->cur_exit_led = enter_led;
        }
    }

    return ret;
}

static std::string proc_new_vehicle_trigger(const std::string &_vehicle_number, cur_enter_vehicle_type _enter_type)
{
    std::string ret;
    switch (_enter_type)
    {
    case enter_type_vehicle_order:
    {
        auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number = '%s' AND status != 100", _vehicle_number.c_str());
        if (vo)
        {
            if (vo->m_called)
            {
                if (vo->status == 3 && !vo->m_permit)
                {
                    ret = "车辆未确认装卸货";
                }
            }
            else
            {
                ret = "车辆未叫号";
            }
        }
        else
        {
            ret = "未找到车辆信息";
        }
        break;
    }
    case enter_type_white_record:
        break;
    case enter_type_no_thing:
        ret = "未找到车辆信息";
        break;
    default:
        break;
    }

    return ret;
}

void ssm_empty::proc_event_vehicle_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_vehicle_number)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    sm.m_face.reset(get_cur_face(_device_type).release());
    auto enter_type = vehicle_is_in_system(_vehicle_number);
    auto fail_reason = proc_new_vehicle_trigger(_vehicle_number, enter_type);
    if (sm.m_face)
    {
        if (fail_reason.length() > 0)
        {
            sm.led_show_and_cast(sm.m_face->cur_enter_led, fail_reason, _vehicle_number);
            sm.m_face.reset();
        }
        else
        {
            sm.send_gate_control_msg(sm.m_face->cur_enter_gate, false);
            sm.led_show_and_cast(sm.m_face->cur_enter_led, "请上磅", _vehicle_number);
            sm.scale_vehicle = _vehicle_number;
            sm.scale_type = enter_type;
        }
    }
}
void ssm_empty::proc_event_vehicle_id_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_id)
{
}
void ssm_empty::proc_event_vehicle_qr_scan(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_qr_code)
{
}
void ssm_empty::proc_event_timeout(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.send_gate_is_close_msg(sm.m_face->cur_enter_gate);
    }
}
void ssm_empty::proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        if (_device_type == sm.m_face->cur_enter_gate && false == _is_close)
        {
            need_next = true;
        }
    }
}
void ssm_empty::after_enter(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    sm.m_face.reset();
    sm.scale_vehicle = "";
    sm.scale_type = enter_type_no_thing;
    sm.cur_scale_weight = 0;
    sm.send_manual_trigger_msg(enter_cam);
    sm.send_manual_trigger_msg(exit_cam);
}
void ssm_empty::before_exit(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.send_take_picture_msg(sm.m_face->cur_enter_cam);
    }
}
std::unique_ptr<abs_state_machine_state> ssm_empty::get_next(abs_state_machine &_sm)
{
    if (need_next)
    {
        return std::unique_ptr<ssm_wait_vehicle_come>(new ssm_wait_vehicle_come());
    }

    return std::unique_ptr<abs_state_machine_state>();
}

void ssm_wait_vehicle_come:: proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        if (_device_type == sm.m_face->cur_enter_gate && _is_close)
        {
            need_next = true;
        }
        else
        {
            sm.led_show_and_cast(enter_led, "车辆未完全上磅", sm.scale_vehicle);
            sm.led_show_and_cast(exit_led, "车辆未完全上磅", sm.scale_vehicle);
        }
    }
}
void ssm_wait_vehicle_come::after_enter(abs_state_machine &_sm)
{
}
void ssm_wait_vehicle_come::before_exit(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.led_show_and_cast(sm.m_face->cur_exit_led, "请保持静止等待提示", sm.scale_vehicle);
    }
}
void ssm_wait_vehicle_come::proc_event_timeout(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.send_gate_is_close_msg(sm.m_face->cur_enter_gate);
    }
}
std::unique_ptr<abs_state_machine_state> ssm_wait_vehicle_come::get_next(abs_state_machine &_sm)
{
    if (need_next)
    {
        return std::unique_ptr<ssm_scale>(new ssm_scale());
    }
    return std::unique_ptr<abs_state_machine_state>();
}

void ssm_scale::proc_event_cur_weight(abs_state_machine &_sm, ssm_device_type _device_type, double _weight)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.led_show(sm.m_face->cur_exit_led, "正在称重", zh_double2string_reserve2(_weight));
    }
    if (_weight > 0)
    {
        if (weight_list.size() > 0)
        {
            auto last_weight = weight_list.back();
            if (last_weight != _weight)
            {
                weight_list.clear();
            }
        }
        weight_list.push_back(_weight);
        if (weight_list.size() >= 3)
        {
            sm.cur_scale_weight = _weight;
            if (sm.scale_type == enter_type_vehicle_order)
            {
                auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", sm.scale_vehicle.c_str());
                if (vo && vo->status == 3)
                {
                    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
                    if (stuff->need_manual_scale && (_weight < stuff->min_limit || _weight > stuff->max_limit))
                    {
                        need_confirm = true;
                    }
                }
            }
            finish_weight = true;
        }
    }
}
void ssm_scale::proc_event_timeout(abs_state_machine &_sm)
{
    auto cur_time = time(nullptr);
    if (cur_time - last_sec > 3)
    {
        last_sec = cur_time;
        auto &sm = dynamic_cast<scale_sm1 &>(_sm);
        sm.send_scale_weight_msg(scale);
    }
}
void ssm_scale::after_enter(abs_state_machine &_sm)
{
    last_sec = time(nullptr);
}
void ssm_scale::before_exit(abs_state_machine &_sm)
{
}
std::unique_ptr<abs_state_machine_state> ssm_scale::get_next(abs_state_machine &_sm)
{
    std::unique_ptr<abs_state_machine_state> ret;

    if (finish_weight)
    {
        ret.reset(new ssm_wait_vehicle_leave());
        if (need_confirm)
        {
            ret.reset(new ssm_confirm());
        }
    }

    return ret;
}

void ssm_confirm::proc_manual_confirm(abs_state_machine &_sm)
{
    need_next = true;
}
void ssm_confirm::after_enter(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.led_show_and_cast(sm.m_face->cur_exit_led, "请联系管理员确认称重", zh_double2string_reserve2(sm.cur_scale_weight));
    }
}
void ssm_confirm::before_exit(abs_state_machine &_sm)
{

}
std::unique_ptr<abs_state_machine_state> ssm_confirm::get_next(abs_state_machine &_sm)
{
    if (need_next)
    {
        return std::unique_ptr<ssm_wait_vehicle_leave>(new ssm_wait_vehicle_leave());
    }
    return std::unique_ptr<abs_state_machine_state>();
}


void ssm_wait_vehicle_leave::proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face && sm.m_face->cur_exit_gate == _device_type && _is_close)
    {
        gate_is_close = true;
    }
    else
    {
        gate_is_close = false;
    }
}

void ssm_wait_vehicle_leave::proc_event_cur_weight(abs_state_machine &_sm, ssm_device_type _device_type, double _weight)
{
    if (_weight == 0)
    {
        weight_is_clear = true;
    }
    else
    {
        weight_is_clear = false;
    }
}
void ssm_wait_vehicle_leave::after_enter(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.send_gate_control_msg(sm.m_face->cur_exit_gate, false);
        sm.led_show_and_cast(sm.m_face->cur_exit_led, "请下磅：" + zh_double2string_reserve2(sm.cur_scale_weight), sm.scale_vehicle);
    }
}
void ssm_wait_vehicle_leave::before_exit(abs_state_machine &_sm)
{

}
void ssm_wait_vehicle_leave::proc_event_timeout(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.send_gate_is_close_msg(sm.m_face->cur_exit_gate);
        sm.send_scale_weight_msg(scale);
    }
}
std::unique_ptr<abs_state_machine_state> ssm_wait_vehicle_leave::get_next(abs_state_machine &_sm)
{
    if (weight_is_clear && gate_is_close)
    {
        return std::unique_ptr<ssm_empty>(new ssm_empty());
    }

    return std::unique_ptr<abs_state_machine_state>();
}

class dida_timer : public epoll_node_t
{
public:
    int timer_fd = -1;
    scale_sm1 *m_pscale = nullptr;
    dida_timer(scale_sm1 &_sm) : m_pscale(&_sm)
    {
        timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (timer_fd >= 0)
        {
            timespec tv = {
                .tv_sec = 1,
                .tv_nsec = 0};
            itimerspec itv = {
                .it_interval = tv,
                .it_value = tv};
            timerfd_settime(timer_fd, 0, &itv, nullptr);
        }
    }
    virtual bool proc_in()
    {
        long sec = 0;
        read(timer_fd, &sec, sizeof(sec));
        if (m_pscale && m_pscale->m_cur_state)
        {
            m_pscale->m_cur_state->proc_event_timeout(*m_pscale);
        }
        return true;
    }
    virtual int get_fd() const
    {
        return timer_fd;
    }
};

int main(int argc, char const *argv[])
{
    scale_sm1 sm(argc, argv, new ssm_empty());
    sm.init_sm();
    dida_timer dt(sm);
    epoll_sch_center sch;
    sch.add_node(sm);
    sch.add_node(dt);
    sch.run();

    return 0;
}
