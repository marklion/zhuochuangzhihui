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
            printf("send msg:%s\n", _msg.c_str());
            mq_send(tmp_fd, _msg.c_str(), _msg.length(), 0);
            mq_close(tmp_fd);
        }
    }
    void proc_msg()
    {
        char buff[9600] = {0};
        unsigned int prio = 0;
        auto recv_len = mq_receive(self_que_fd, buff, sizeof(buff), &prio);
        if (recv_len > 0)
        {
            printf("recv event: %s\n", buff);
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
        }
    }
};

#endif // _STATE_MACHINE_COMMON_H_
