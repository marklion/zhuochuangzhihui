#include "../state_machine_common.h"

class scale_sm1 : public abs_state_machine
{
public:
    using abs_state_machine::abs_state_machine;
    virtual void proc_event_vehicle_come(ssm_device_type _device_type, const std::string &_vehicle_number){
        PRINT_LOG("plate is %s", _vehicle_number.c_str());
        send_gate_is_close_msg(_device_type);
        send_take_picture_msg(_device_type);
        if (time(nullptr)%10 == 0)
        {
            sleep(1);
            send_manual_trigger_msg(_device_type);
        }
        send_led_show_msg(_device_type, _vehicle_number, "嘿嘿", "123123123123123", "ddd");
    };
    virtual void proc_event_vehicle_id_come(ssm_device_type _device_type, const std::string &_id){};
    virtual void proc_event_vehicle_qr_scan(ssm_device_type _device_type, const std::string &_qr_code)
    {
        send_printer_print_msg(enter_printer, "aaaaa", _qr_code);
    };
    virtual void proc_event_picture_resp(ssm_device_type _device_type, const std::string &_picture){
        PRINT_LOG("pic path:https://%s%s", getenv("BASE_URL"), _picture.c_str());
    };
    virtual void proc_event_video_record_resp(ssm_device_type _device_type, const std::string &_video){};
    virtual void proc_event_gate_is_close(ssm_device_type _device_type, bool _is_close)
    {
        if (_is_close)
        {
            PRINT_LOG("gate is close");
        }
        else
        {
            PRINT_LOG("gate is open");
        }
        send_gate_control_msg(_device_type, !_is_close);
    };
    virtual void proc_event_cur_weight(ssm_device_type _device_type, double _weight){};
};

int main(int argc, char const *argv[])
{
    scale_sm1 sm(argc, argv);
    while (1)
    {
        sm.proc_msg();
    }

    return 0;
}
