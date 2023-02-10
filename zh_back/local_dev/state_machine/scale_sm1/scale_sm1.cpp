#include "../state_machine_common.h"

class scale_sm1 : public abs_state_machine
{
public:
    using abs_state_machine::abs_state_machine;
    virtual void proc_event_vehicle_come(ssm_device_type _device_type, const std::string &_vehicle_number)
    {
        std::cout << "vehicle come from" << _device_type << ":" << _vehicle_number << std::endl;
        neb::CJsonObject tmp;
        tmp.Add("msg_type", LOCAL_DEV_MSG_TAKE_PICTURE);
        send_msg(_device_type, tmp.ToString());
    };
    virtual void proc_event_vehicle_id_come(ssm_device_type _device_type, const std::string &_id){};
    virtual void proc_event_vehicle_qr_scan(ssm_device_type _device_type, const std::string &_qr_code){};
    virtual void proc_event_picture_resp(ssm_device_type _device_type, const std::string &_picture)
    {
        std::cout << "picture from" << _device_type << ":" << _picture << std::endl;
    };
    virtual void proc_event_video_record_resp(ssm_device_type _device_type, const std::string &_video){};
    virtual void proc_event_gate_is_close(ssm_device_type _device_type, bool _is_close){};
    virtual void proc_event_cur_weight(ssm_device_type _device_type, double _weight){};
};

int main(int argc, char const *argv[])
{
    scale_sm1 sm(argc, argv);
    while (1)
    {
        sm.proc_msg();
        std::cout << "event comeing " << std::endl;
    }

    return 0;
}
