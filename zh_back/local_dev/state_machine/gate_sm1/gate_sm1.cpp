#include "../state_machine_common.h"
class gate_sm1 : public abs_gate_state_machine
{
public:
    using abs_gate_state_machine::abs_gate_state_machine;

    virtual void proc_event_picture_resp(ssm_device_type _device_type, const std::string &_picture)
    {
        save_picture_by_vehicle(_picture);
        record_vehicle_action();
        pass_vehicle = "";
    }
};
class uniq_gate_state : public abs_state_machine_state
{
    ssm_device_type get_cur_gate(ssm_device_type _cur_device)
    {
        ssm_device_type ret = exit_gate;
        if (_cur_device == enter_cam || _cur_device == enter_id || _cur_device == enter_qr)
        {
            ret = enter_gate;
        }

        return ret;
    }
    ssm_device_type get_cur_led(ssm_device_type _cur_device)
    {
        ssm_device_type ret = exit_led;
        if (_cur_device == enter_cam || _cur_device == enter_id || _cur_device == enter_qr)
        {
            ret = enter_led;
        }

        return ret;
    }

public:
    virtual void proc_event_vehicle_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_vehicle_number)
    {
        auto enter_type = vehicle_is_in_system(_vehicle_number);
        auto fail_reason = proc_new_vehicle_trigger(_vehicle_number, enter_type, true);
        if (fail_reason.length() > 0)
        {
            _sm.led_show_and_cast(get_cur_led(_device_type), fail_reason, _vehicle_number);
        }
        else
        {
            std::string cast_content = "请入场";
            if (_device_type != enter_cam)
            {
                cast_content = "一路顺风";
            }
            _sm.send_gate_control_msg(get_cur_gate(_device_type), false);
            _sm.led_show_and_cast(get_cur_led(_device_type), cast_content, _vehicle_number);
            _sm.send_take_picture_msg(_device_type);

            auto &sm = dynamic_cast<gate_sm1 &>(_sm);
            sm.pass_vehicle = _vehicle_number;
        }
    };
    virtual void proc_event_vehicle_id_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_id){};
    virtual void proc_event_vehicle_qr_scan(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_qr_code){};
    virtual void after_enter(abs_state_machine &_sm){};
    virtual void before_exit(abs_state_machine &_sm){};
};

int main(int argc, char const *argv[])
{
    gate_sm1 sm(argc, argv, new uniq_gate_state());
    sm.init_sm();
    while (1)
    {
        sm.proc_msg();
    }

    return 0;
}
