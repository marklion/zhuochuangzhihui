#include "../state_machine_common.h"
#include "scale_sm1.h"
class ssm_wait_vehicle_come;
class scale_sm1 : public abs_state_machine, public epoll_node_t
{
public:
    using abs_state_machine::abs_state_machine;
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
void ssm_empty::proc_event_vehicle_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_vehicle_number)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    sm.send_led_show_msg(_device_type, _vehicle_number, "", "", "");
    sm.send_speaker_speak_msg(_device_type, _vehicle_number + "来了");
    sm.send_take_picture_msg(_device_type);
    sm.send_gate_control_msg(_device_type, false);
    sm.send_gate_is_close_msg(_device_type);
};
void ssm_empty::proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close)
{
    if (!_is_close)
    {
        need_next = true;
    }
    else
    {
        auto &sm = dynamic_cast<scale_sm1 &>(_sm);
        sm.send_gate_is_close_msg(_device_type);
    }
};
void ssm_empty::after_enter(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    std::string state_string = "进入空闲";
    sm.send_led_show_msg(ssm_device_type::enter_led, "卓创智汇", state_string, "", "");
    sm.send_led_show_msg(ssm_device_type::exit_led, "卓创智汇", state_string, "", "");
    sm.send_speaker_speak_msg(ssm_device_type::enter_speaker, state_string);
    sm.send_speaker_speak_msg(ssm_device_type::exit_speaker, state_string);
    sm.send_triffic_light_msg(ssm_device_type::enter_traffic_light, true);
    sm.send_triffic_light_msg(ssm_device_type::exit_traffic_light, true);
    sm.send_gate_control_msg(ssm_device_type::enter_gate, true);
    sm.send_gate_control_msg(ssm_device_type::exit_gate, true);
};
void ssm_empty::before_exit(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    std::string state_string = "离开空闲";
    sm.send_led_show_msg(ssm_device_type::enter_led, "卓创智汇", state_string, "", "");
    sm.send_led_show_msg(ssm_device_type::exit_led, "卓创智汇", state_string, "", "");
    sm.send_speaker_speak_msg(ssm_device_type::enter_speaker, state_string);
    sm.send_speaker_speak_msg(ssm_device_type::exit_speaker, state_string);
};
std::unique_ptr<abs_state_machine_state> ssm_empty::get_next(abs_state_machine &_sm)
{
    if (need_next)
    {
        return std::unique_ptr<ssm_wait_vehicle_come>(new ssm_wait_vehicle_come());
    }
    return nullptr;
};
void ssm_wait_vehicle_come::proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close)
{
    if (_is_close)
    {
        need_next = true;
    }
    else
    {
        auto &sm = dynamic_cast<scale_sm1 &>(_sm);
        sm.send_gate_is_close_msg(_device_type);
    }
};
void ssm_wait_vehicle_come::after_enter(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    std::string state_string = "进入上车";
    sm.send_led_show_msg(ssm_device_type::enter_led, "卓创智汇", state_string, "", "");
    sm.send_led_show_msg(ssm_device_type::exit_led, "卓创智汇", state_string, "", "");
    sm.send_speaker_speak_msg(ssm_device_type::enter_speaker, state_string);
    sm.send_speaker_speak_msg(ssm_device_type::exit_speaker, state_string);
    sm.send_triffic_light_msg(ssm_device_type::enter_traffic_light, false);
    sm.send_triffic_light_msg(ssm_device_type::exit_traffic_light, false);
    sm.send_gate_control_msg(ssm_device_type::enter_gate, true);
    sm.send_gate_control_msg(ssm_device_type::exit_gate, true);
    sm.send_gate_is_close_msg(ssm_device_type::exit_gate);
};
void ssm_wait_vehicle_come::before_exit(abs_state_machine &_sm)
{
    auto &sm = dynamic_cast<scale_sm1 &>(_sm);
    std::string state_string = "离开上车";
    sm.send_led_show_msg(ssm_device_type::enter_led, "卓创智汇", state_string, "", "");
    sm.send_led_show_msg(ssm_device_type::exit_led, "卓创智汇", state_string, "", "");
    sm.send_speaker_speak_msg(ssm_device_type::enter_speaker, state_string);
    sm.send_speaker_speak_msg(ssm_device_type::exit_speaker, state_string);
};
std::unique_ptr<abs_state_machine_state> ssm_wait_vehicle_come::get_next(abs_state_machine &_sm)
{
    if (need_next)
    {
        return std::unique_ptr<ssm_empty>(new ssm_empty());
    }
    return nullptr;
};

void ssm_wait_vehicle_come::proc_event_picture_resp(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_picture)
{
    PRINT_LOG("pic path:%s", _picture);
}
int main(int argc, char const *argv[])
{
    scale_sm1 sm(argc, argv, new ssm_empty());
    sm.init_sm();
    epoll_sch_center sch;
    sch.add_node(sm);
    sch.run();

    return 0;
}
