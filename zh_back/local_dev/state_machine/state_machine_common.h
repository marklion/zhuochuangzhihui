#if !defined(_STATE_MACHINE_COMMON_H_)
#define _STATE_MACHINE_COMMON_H_

#include "../utils/local_utils.h"
#include "../../zh_database/zh_db_config.h"

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

class abs_state_machine;
class abs_state_machine_state{
public:
    virtual void proc_event_vehicle_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_vehicle_number){};
    virtual void proc_event_vehicle_id_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_id){};
    virtual void proc_event_vehicle_qr_scan(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_qr_code){};
    virtual void proc_event_picture_resp(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_picture){};
    virtual void proc_event_video_record_resp(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_video){};
    virtual void proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close){};
    virtual void proc_event_cur_weight(abs_state_machine &_sm, ssm_device_type _device_type, double _weight){};
    virtual void proc_event_timeout(abs_state_machine &_sm) {};
    virtual void after_enter(abs_state_machine &_sm) {};
    virtual void before_exit(abs_state_machine &_sm) {};
    virtual void proc_manual_confirm(abs_state_machine &_sm) {};
    virtual std::unique_ptr<abs_state_machine_state> get_next(abs_state_machine &_sm) {
        return nullptr;
    };
};

class abs_state_machine
{
protected:
    std::map<ssm_device_type, std::list<std::string>> type_pic_req_map;
    std::map<ssm_device_type, std::list<std::string>> type_video_req_map;
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
    std::unique_ptr<abs_state_machine_state> m_cur_state;
    abs_state_machine(int argc, const char *const *argv, abs_state_machine_state *_init_state):m_cur_state(_init_state)
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
    void trans_msg_to(const std::string &_reciever, const std::string &_msg)
    {
        auto tmp_fd = mq_open(("/" + _reciever).c_str(), O_WRONLY);
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
    void init_sm()
    {
        m_cur_state->after_enter(*this);
    }
    void sm_change() {
        auto next_state = m_cur_state->get_next(*this);
        if (next_state)
        {
            m_cur_state->before_exit(*this);
            m_cur_state.reset(next_state.release());
            m_cur_state->after_enter(*this);
        }
    }
    ssm_device_type get_device_type_by_msg_type(const std::string &msg_type, const std::string &_device_name)
    {
        ssm_device_type ret = (ssm_device_type)-1;
        if (LOCAL_DEV_EVENT_GATE_IS_CLOSE == msg_type)
        {
            auto tmp = enter_gate;
            if (type_name_map[tmp] == _device_name)
            {
                ret = tmp;
            }
            else
            {
                ret = exit_gate;
            }
        }
        else if (LOCAL_DEV_EVENT_ID_COME == msg_type)
        {
            auto tmp = enter_id;
            if (type_name_map[tmp] == _device_name)
            {
                ret = tmp;
            }
            else
            {
                ret = exit_id;
            }
        }
        else if (LOCAL_DEV_EVENT_QR_SCAN == msg_type)
        {
            auto tmp = enter_qr;
            if (type_name_map[tmp] == _device_name)
            {
                ret = tmp;
            }
            else
            {
                ret = exit_qr;
            }
        }
        else if (LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT == msg_type)
        {
            ret = scale;
        }
        else if (LOCAL_DEV_EVENT_TAKE_PICTURE == msg_type)
        {
            auto tmp = enter_cam;
            if (type_name_map[tmp] == _device_name)
            {
                ret = tmp;
            }
            else
            {
                ret = exit_cam;
            }
        }
        else if (LOCAL_DEV_EVENT_GET_VIDEO_RECORD == msg_type)
        {
            auto tmp = enter_monitor;
            if (type_name_map[tmp] == _device_name)
            {
                ret = tmp;
            }
            else
            {
                ret = exit_monitor;
            }
        }
        else if (LOCAL_DEV_EVENT_VEHICLE_COME == msg_type)
        {
            auto tmp = enter_cam;
            if (type_name_map[tmp] == _device_name)
            {
                ret = tmp;
            }
            else
            {
                ret = exit_cam;
            }
        }
        return ret;
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
            ssm_device_type device_type = get_device_type_by_msg_type(msg_type, device_name);
            if (-1 != device_type)
            {
                if (LOCAL_DEV_EVENT_GATE_IS_CLOSE == msg_type)
                {
                    bool is_close = false;
                    msg.Get(LOCAL_DEV_EVENT_GATE_IS_CLOSE_KEY, is_close);
                    m_cur_state->proc_event_gate_is_close(*this, device_type, is_close);
                    proc_event_gate_is_close(device_type, is_close);
                }
                else if (LOCAL_DEV_EVENT_ID_COME == msg_type)
                {
                    auto id = msg(LOCAL_DEV_EVENT_ID_COME_KEY);
                    m_cur_state->proc_event_vehicle_id_come(*this, device_type, id);
                    proc_event_vehicle_id_come(device_type, id);
                }
                else if (LOCAL_DEV_EVENT_QR_SCAN == msg_type)
                {
                    auto qr_code = msg(LOCAL_DEV_EVENT_QR_SCAN_KEY);
                    m_cur_state->proc_event_vehicle_qr_scan(*this, device_type, qr_code);
                    proc_event_vehicle_qr_scan(device_type, qr_code);
                }
                else if (LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT == msg_type)
                {
                    double weight = 0;
                    msg.Get(LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT_KEY, weight);
                    m_cur_state->proc_event_cur_weight(*this, device_type, weight);
                    proc_event_cur_weight(device_type, weight);
                }
                else if (LOCAL_DEV_EVENT_TAKE_PICTURE == msg_type)
                {
                    auto picture = msg(LOCAL_DEV_EVENT_TAKE_PICTURE_KEY);
                    m_cur_state->proc_event_picture_resp(*this, device_type, picture);
                    proc_event_picture_resp(device_type, picture);
                    auto &pic_req_list = type_pic_req_map[device_type];
                    if (pic_req_list.size() > 0)
                    {
                        auto reciever = pic_req_list.front();
                        pic_req_list.pop_front();
                        trans_msg_to(reciever, msg.ToString());
                    }
                }
                else if (LOCAL_DEV_EVENT_GET_VIDEO_RECORD == msg_type)
                {
                    auto video = msg(LOCAL_DEV_EVENT_GET_VIDEO_RECORD_KEY);
                    m_cur_state->proc_event_video_record_resp(*this, device_type, video);
                    proc_event_video_record_resp(device_type, video);
                    auto &video_req_list = type_video_req_map[device_type];
                    if (video_req_list.size() > 0)
                    {
                        auto reciever = video_req_list.front();
                        video_req_list.pop_front();
                        trans_msg_to(reciever, msg.ToString());
                    }
                }
                else if (LOCAL_DEV_EVENT_VEHICLE_COME == msg_type)
                {
                    auto vehicle_nubmer = msg(LOCAL_DEV_EVENT_VEHICLE_COME_KEY);
                    m_cur_state->proc_event_vehicle_come(*this, device_type, vehicle_nubmer);
                    proc_event_vehicle_come(device_type, vehicle_nubmer);
                }

            }
            else
            {
                if (LOCAL_DEV_REQ_TAKE_PICTURE == msg_type)
                {
                    bool is_enter = false;
                    msg.Get(LOCAL_DEV_REQ_TAKE_PICTURE_KEY, is_enter);
                    auto cam = enter_cam;
                    if (!is_enter)
                    {
                        cam = exit_cam;
                    }
                    send_take_picture_msg(cam);
                    type_pic_req_map[cam].push_back(device_name);
                }
                else if (LOCAL_DEV_REQ_TAKE_VIDEO == msg_type)
                {
                    bool is_enter = false;
                    msg.Get(LOCAL_DEV_REQ_TAKE_VIDEO_KEY1, is_enter);
                    auto begin_date = msg(LOCAL_DEV_REQ_TAKE_VIDEO_KEY2);
                    auto end_date = msg(LOCAL_DEV_REQ_TAKE_VIDEO_KEY3);
                    auto cam = enter_monitor;
                    if (!is_enter)
                    {
                        cam = exit_monitor;
                    }
                    send_get_video_record_msg(cam, begin_date, end_date);
                    type_video_req_map[cam].push_back(device_name);
                }
                else if (LOCAL_DEV_EVENT_DEVICE_STATUS == msg_type)
                {
                    bool healthy;
                    msg.Get(LOCAL_DEV_EVENT_DEVICE_STATUS_KEY, healthy);
                    for (auto itr = type_name_map.begin(); itr != type_name_map.end(); ++itr)
                    {
                        if (itr->second == device_name)
                        {
                            device_health_map[itr->first] = healthy;
                        }
                    }
                }
                else
                {
                    PRINT_ERR("recv msg from %s, but it has not registed to this sm", device_name.c_str());
                }
            }
        }
    }
};
struct focus_face_set
{
    ssm_device_type cur_enter_gate;
    ssm_device_type cur_enter_cam;
    ssm_device_type cur_enter_led;
    ssm_device_type cur_exit_gate;
    ssm_device_type cur_exit_cam;
    ssm_device_type cur_exit_led;
};
enum cur_enter_vehicle_type
{
    enter_type_vehicle_order,
    enter_type_white_record,
    enter_type_no_thing,
};
class abs_scale_state_machine : public abs_state_machine
{
public:
    using abs_state_machine::abs_state_machine;
    cur_enter_vehicle_type scale_type = enter_type_no_thing;
    std::string scale_vehicle;
    double cur_scale_weight = 0;
    std::unique_ptr<focus_face_set> m_face;
};

#endif // _STATE_MACHINE_COMMON_H_
