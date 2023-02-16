#if !defined(_STATE_MACHINE_COMMON_H_)
#define _STATE_MACHINE_COMMON_H_

#include "../utils/local_utils.h"

enum ssm_device_type
{
    enter_cam,
    exit_cam,
    enter_led,
    exit_led,
    enter_traffic_light,
    exit_traffic_light,
    enter_speaker,
    exit_speaker,
    enter_id,
    exit_id,
    enter_qr,
    exit_qr,
    enter_printer,
    exit_printer,
    enter_monitor,
    exit_monitor,
    enter_gate,
    exit_gate,
    scale
};

class abs_state_machine
{
protected:
    int self_que_fd = -1;
    std::map<ssm_device_type, std::string> type_name_map;
    std::map<ssm_device_type, bool> device_health_map;
    virtual void proc_event_vehicle_come(ssm_device_type _device_type, const std::string &_vehicle_number){};
    virtual void proc_event_vehicle_id_come(ssm_device_type _device_type, const std::string &_id){};
    virtual void proc_event_vehicle_qr_scan(ssm_device_type _device_type, const std::string &_qr_code){};
    virtual void proc_event_picture_resp(ssm_device_type _device_type, const std::string &_picture){};
    virtual void proc_event_video_record_resp(ssm_device_type _device_type, const std::string &_video){};
    virtual void proc_event_gate_is_close(ssm_device_type _device_type, bool _is_close){};
    virtual void proc_event_cur_weight(ssm_device_type _device_type, double _weight){};
public:
    abs_state_machine(int argc, const char *const *argv)
    {
        std::string self_que_name = "/" + std::string(argv[0]);
        auto tmp_fd = mq_open(self_que_name.c_str(), O_RDONLY);
        if (tmp_fd >= 0)
        {
            self_que_fd = tmp_fd;
        }
        for (auto i = 1; i < argc; i++)
        {
            type_name_map[(ssm_device_type)(i-1)] = argv[i];
            device_health_map[(ssm_device_type)(i-1)] = true;
        }
    }
    virtual ~abs_state_machine()
    {
        if (self_que_fd >= 0)
        {
            mq_close(self_que_fd);
        }
    }
    void send_msg(ssm_device_type _device, const std::string &_msg)
    {
        auto recv_device =  "/" + type_name_map[_device];
        auto tmp_fd = mq_open(recv_device.c_str(), O_WRONLY);
        if (tmp_fd >= 0)
        {
            PRINT_LOG("send msg:%s", _msg.c_str());
            mq_send(tmp_fd, _msg.c_str(), _msg.length(), 0);
            mq_close(tmp_fd);
        }
    }
    void send_take_picture_msg(ssm_device_type _device)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_TAKE_PICTURE);
        send_msg(_device, msg.ToString());
    }
    void send_manual_trigger_msg(ssm_device_type _device)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_MANUAL_TRIIGER);
        send_msg(_device, msg.ToString());
    }
    void send_led_show_msg(ssm_device_type _device, const std::string &_l1, const std::string &_l2, const std::string &_l3, const std::string &_l4)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_LED_SHOW);
        msg.Add(LOCAL_DEV_MSG_LED_SHOW_KEY1, _l1);
        msg.Add(LOCAL_DEV_MSG_LED_SHOW_KEY2, _l2);
        msg.Add(LOCAL_DEV_MSG_LED_SHOW_KEY3, _l3);
        msg.Add(LOCAL_DEV_MSG_LED_SHOW_KEY4, _l4);
        send_msg(_device, msg.ToString());
    }
    void send_speaker_speak_msg(ssm_device_type _device, const std::string &_voice)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_SPEAKER_SPEAK);
        msg.Add(LOCAL_DEV_MSG_SPEAKER_SPEAK_KEY, _voice);
        send_msg(_device, msg.ToString());
    }
    void send_triffic_light_msg(ssm_device_type _device, bool _is_green)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_TRAFFIC_LIGHT_SET);
        msg.Add(LOCAL_DEV_MSG_TRAFFIC_LIGHT_SET_KEY, _is_green, _is_green);
        send_msg(_device, msg.ToString());
    }
    void send_get_video_record_msg(ssm_device_type _device, const std::string &_begin_date, const std::string &_end_date)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_GET_VIDEO_RECORD);
        msg.Add(LOCAL_DEV_MSG_GET_VIDEO_RECORD_KEY1, _begin_date);
        msg.Add(LOCAL_DEV_MSG_GET_VIDEO_RECORD_KEY2, _end_date);
        send_msg(_device, msg.ToString());
    }
    void send_gate_control_msg(ssm_device_type _device, bool _is_close)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_GATE_CONTROL);
        msg.Add(LOCAL_DEV_MSG_GATE_CONTROL_KEY, _is_close, _is_close);
        send_msg(_device, msg.ToString());
    }
    void send_gate_is_close_msg(ssm_device_type _device)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_GATE_IS_CLOSE);
        send_msg(_device, msg.ToString());
    }
    void send_printer_print_msg(ssm_device_type _device, const std::string &_content, const std::string &_qr_code)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_PRINTER_PRINT);
        msg.Add(LOCAL_DEV_MSG_PRINTER_PRINT_KEY1,  _content);
        msg.Add(LOCAL_DEV_MSG_PRINTER_PRINT_KEY2,  _qr_code);
        send_msg(_device, msg.ToString());
    }
    void send_scale_weight_msg(ssm_device_type _device)
    {
        neb::CJsonObject msg;
        msg.Add("msg_type", LOCAL_DEV_MSG_SCALE_CUR_WEIGHT);
        send_msg(_device, msg.ToString());
    }
    void proc_msg()
    {
        char buff[9600] = {0};
        unsigned int prio = 0;
        auto recv_len = mq_receive(self_que_fd, buff, sizeof(buff), &prio);
        if (recv_len > 0)
        {
            PRINT_LOG("recv event: %s", buff);
            // 解析报文获取消息对象
            neb::CJsonObject msg(std::string(buff, recv_len));
            auto msg_type = msg("msg_type");
            auto device_name = msg("device_name");
            ssm_device_type device_type = (ssm_device_type)-1;
            for (auto itr = type_name_map.begin(); itr!= type_name_map.end(); ++itr)
            {
                if (itr->second == device_name)
                {
                    device_type = itr->first;
                }
            }
            if (-1 != device_type)
            {
                if (LOCAL_DEV_EVENT_GATE_IS_CLOSE == msg_type)
                {
                    bool is_close = false;
                    msg.Get(LOCAL_DEV_EVENT_GATE_IS_CLOSE_KEY, is_close);
                    proc_event_gate_is_close(device_type, is_close);
                }
                else if (LOCAL_DEV_EVENT_ID_COME == msg_type)
                {
                    auto id = msg(LOCAL_DEV_EVENT_ID_COME_KEY);
                    proc_event_vehicle_id_come(device_type, id);
                }
                else if (LOCAL_DEV_EVENT_QR_SCAN == msg_type)
                {
                    auto qr_code = msg(LOCAL_DEV_EVENT_QR_SCAN_KEY);
                    proc_event_vehicle_qr_scan(device_type, qr_code);
                }
                else if (LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT == msg_type)
                {
                    double weight = 0;
                    msg.Get(LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT_KEY, weight);
                    proc_event_cur_weight(device_type, weight);
                }
                else if (LOCAL_DEV_EVENT_TAKE_PICTURE == msg_type)
                {
                    auto picture = msg(LOCAL_DEV_EVENT_TAKE_PICTURE_KEY);
                    proc_event_picture_resp(device_type, picture);
                }
                else if (LOCAL_DEV_EVENT_GET_VIDEO_RECORD == msg_type)
                {
                    auto video = msg(LOCAL_DEV_EVENT_GET_VIDEO_RECORD_KEY);
                    proc_event_video_record_resp(device_type, video);
                }
                else if (LOCAL_DEV_EVENT_VEHICLE_COME == msg_type)
                {
                    auto vehicle_nubmer = msg(LOCAL_DEV_EVENT_VEHICLE_COME_KEY);
                    proc_event_vehicle_come(device_type, vehicle_nubmer);
                }
                else if (LOCAL_DEV_EVENT_DEVICE_STATUS == msg_type)
                {
                    bool healthy;
                    msg.Get(LOCAL_DEV_EVENT_DEVICE_STATUS_KEY, healthy);
                    device_health_map[device_type] = healthy;
                    PRINT_LOG("recv %d status:%d", device_type, healthy);
                }
            }
            else
            {
                PRINT_ERR("recv msg from %s, but it has not registed to this sm", device_name.c_str());
            }
        }
    }
};

#endif // _STATE_MACHINE_COMMON_H_
