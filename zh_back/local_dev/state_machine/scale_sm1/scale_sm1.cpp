#include "../state_machine_common.h"
#include "scale_sm1.h"

class scale_sm1 : public abs_scale_state_machine, public epoll_node_t
{
public:
    using abs_scale_state_machine::abs_scale_state_machine;
    virtual bool proc_in()
    {
        proc_msg();
        sm_change();
        return true;
    }
    virtual void proc_event_picture_resp(ssm_device_type _device_type, const std::string &_picture){
        if (m_face && m_face->cur_enter_cam == _device_type && scale_vehicle.length() > 0)
        {
            save_picture_by_vehicle(_picture);
        }
    }
    virtual int get_fd() const
    {
        return self_que_fd;
    }
};

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
            ret->cur_enter_printer = enter_printer;
            ret->cur_exit_printer = exit_printer;
        }
        else
        {
            ret->cur_enter_cam = exit_cam;
            ret->cur_enter_gate = exit_gate;
            ret->cur_enter_led = exit_led;
            ret->cur_exit_cam = enter_cam;
            ret->cur_exit_gate = enter_gate;
            ret->cur_exit_led = enter_led;
            ret->cur_enter_printer = exit_printer;
            ret->cur_exit_printer = enter_printer;
        }
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
            sm.send_take_picture_msg(sm.m_face->cur_enter_cam);
            sm.scale_vehicle = _vehicle_number;
            sm.scale_type = enter_type;
        }
    }
}
void ssm_empty::proc_event_vehicle_id_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_id)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    sm.m_face.reset(get_cur_face(_device_type).release());
    if (sm.m_face)
    {
        auto vehicle_number = get_vehicle_by_id(_id);
        if (vehicle_number.length() > 0)
        {
            auto enter_type = vehicle_is_in_system(vehicle_number);
            auto fail_reason = proc_new_vehicle_trigger(vehicle_number, enter_type);
            if (fail_reason.length() > 0)
            {
                sm.led_show_and_cast(sm.m_face->cur_enter_led, fail_reason, vehicle_number);
                sm.m_face.reset();
            }
            else
            {
                sm.send_gate_control_msg(sm.m_face->cur_enter_gate, false);
                sm.led_show_and_cast(sm.m_face->cur_enter_led, "请上磅", vehicle_number);
                sm.send_take_picture_msg(sm.m_face->cur_enter_cam);
                sm.scale_vehicle = vehicle_number;
                sm.scale_type = enter_type;
            }
        }
        else
        {
            sm.led_show_and_cast(sm.m_face->cur_enter_led, "未找到车辆", "");
            sm.m_face.reset();
        }
    }
}
void ssm_empty::proc_event_vehicle_qr_scan(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_qr_code)
{
    auto printer = enter_printer;
    if (_device_type == exit_qr)
    {
        printer = exit_printer;
    }
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    std::string orig_order_number = _qr_code.substr(_qr_code.find_last_of('/') + 1);
    std::string order_number;
    for (auto &itr:orig_order_number)
    {
        if (itr >= '0' && itr <= '9')
        {
            order_number.push_back(itr);
        }
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", order_number.c_str());
    if (vo)
    {
        sm.send_printer_print_msg(printer, order_ticket_content(*vo), "");
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
    sm.send_gate_control_msg(enter_gate, true);
    sm.send_gate_control_msg(exit_gate, true);
}
void ssm_empty::before_exit(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        sm.send_triffic_light_msg(enter_traffic_light, false);
        sm.send_triffic_light_msg(exit_traffic_light, false);
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

void ssm_wait_vehicle_come::proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    if (sm.m_face)
    {
        if (_device_type == sm.m_face->cur_enter_gate)
        {
            if (_is_close)
            {
                need_next = true;
            }
            else
            {
                if (cast_count_down == 0)
                {
                    sm.led_show_and_cast(enter_led, "车辆未完全上磅", sm.scale_vehicle);
                    sm.led_show_and_cast(exit_led, "车辆未完全上磅", sm.scale_vehicle);
                    cast_count_down = 5;
                }
                else
                {
                    cast_count_down--;
                }
            }
        }
    }
}

void ssm_wait_vehicle_come::proc_manual_confirm(abs_state_machine &_sm)
{
    need_next = true;
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
            if (sm.vehicle_need_confirm(_weight))
            {
                need_confirm = true;
            }
            finish_weight = true;
        }
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
    if (sm.m_face && sm.m_face->cur_exit_gate == _device_type)
    {
        gate_is_close = _is_close;
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
        sm.record_vehicle_action();
    }
}
void ssm_wait_vehicle_leave::before_exit(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    sm.send_triffic_light_msg(enter_traffic_light, true);
    sm.send_triffic_light_msg(exit_traffic_light, true);
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
        if (m_pscale)
        {
            m_pscale->proc_event_timeout();
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
    sleep(5);
    return 0;
}
