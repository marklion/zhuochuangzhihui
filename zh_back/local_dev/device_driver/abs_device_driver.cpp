#include "abs_device_driver.h"

abs_device_driver::abs_device_driver(int argc, const char *const *argv, const std::vector<std::string> &_device_arg_param)
{
    self_name = argv[0];
    std::string tmp_center_name(argv[1]);

    auto tmp_fd = mq_open(("/" + self_name).c_str(), O_RDONLY);
    if (tmp_fd >= 0)
    {
        fd = tmp_fd;
        center_name = "/" + tmp_center_name;
    }
    int i = 2;
    for (auto &itr:_device_arg_param)
    {
        if (i < argc)
        {
            device_args[itr] = argv[i];
        }
        else
        {
            break;
        }
        i++;
    }
}

std::string abs_device_driver::get_device_arg(const std::string &_arg_key) const
{
    std::string ret;
    if (device_args.find(_arg_key) != device_args.end())
    {
        ret = device_args.find(_arg_key)->second;
    }

    return ret;
}
abs_device_driver::~abs_device_driver()
{
    if (fd >= 0)
    {
        mq_close(fd);
    }
}

void abs_device_driver::send_event(const std::string &_event)
{
    auto tmp_fd = mq_open(center_name.c_str(), O_WRONLY);
    if (tmp_fd >= 0)
    {
        PRINT_LOG("send event:%s", _event.c_str());
        neb::CJsonObject tmp(_event);
        tmp.Add("device_name", self_name);
        mq_send(tmp_fd, tmp.ToString().c_str(), tmp.ToString().length(), 0);
        mq_close(tmp_fd);
    }
}

void abs_device_driver::pub_vehicle_come(const std::string &_vehicle_number)
{
    neb::CJsonObject event_content;
    event_content.Add("msg_type", LOCAL_DEV_EVENT_VEHICLE_COME);
    event_content.Add(LOCAL_DEV_EVENT_VEHICLE_COME_KEY, _vehicle_number);
    send_event(event_content.ToString());
}
void abs_device_driver::pub_id_come(const std::string &_id_no)
{
    neb::CJsonObject event_content;
    event_content.Add("msg_type", LOCAL_DEV_EVENT_ID_COME);
    event_content.Add(LOCAL_DEV_EVENT_ID_COME_KEY, _id_no);
    send_event(event_content.ToString());
}
void abs_device_driver::pub_qr_scan(const std::string &_qr_code)
{
    neb::CJsonObject event_content;
    event_content.Add("msg_type", LOCAL_DEV_EVENT_QR_SCAN);
    event_content.Add(LOCAL_DEV_EVENT_QR_SCAN_KEY, _qr_code);
    send_event(event_content.ToString());
}

void abs_device_driver::pub_device_status(bool _is_healthy)
{
    neb::CJsonObject event_content;
    event_content.Add("msg_type", LOCAL_DEV_EVENT_DEVICE_STATUS);
    event_content.Add(LOCAL_DEV_EVENT_DEVICE_STATUS_KEY, _is_healthy, _is_healthy);
    send_event(event_content.ToString());
}
int abs_device_driver::get_fd() const
{
    return fd;
}

void abs_device_driver::proc_msg()
{
    char buff[9600] = {0};
    unsigned int prio = 0;
    auto recv_len = mq_receive(fd, buff, sizeof(buff), &prio);
    if (recv_len > 0)
    {
        PRINT_LOG("recv msg:%s", buff);
        // 解析报文获取消息对象
        neb::CJsonObject msg(std::string(buff, recv_len));
        auto msg_type = msg("msg_type");
        if (LOCAL_DEV_MSG_TAKE_PICTURE == msg_type)
        {
            auto resp = take_picture();
            neb::CJsonObject event_content;
            event_content.Add("msg_type", LOCAL_DEV_EVENT_TAKE_PICTURE);
            event_content.Add(LOCAL_DEV_EVENT_TAKE_PICTURE_KEY, resp);
            send_event(event_content.ToString());
        }
        else if (LOCAL_DEV_MSG_MANUAL_TRIIGER == msg_type)
        {
            manual_trigger();
        }
        else if (LOCAL_DEV_MSG_LED_SHOW == msg_type)
        {
            auto l1 = msg(LOCAL_DEV_MSG_LED_SHOW_KEY1);
            auto l2 = msg(LOCAL_DEV_MSG_LED_SHOW_KEY2);
            auto l3 = msg(LOCAL_DEV_MSG_LED_SHOW_KEY3);
            auto l4 = msg(LOCAL_DEV_MSG_LED_SHOW_KEY4);
            led_show(l1, l2, l3, l4);
        }
        else if (LOCAL_DEV_MSG_SPEAKER_SPEAK == msg_type)
        {
            auto voice_content = msg(LOCAL_DEV_MSG_SPEAKER_SPEAK_KEY);
            speaker_speak(voice_content);
        }
        else if (LOCAL_DEV_MSG_TRAFFIC_LIGHT_SET == msg_type)
        {
            bool is_green = false;
            msg.Get(LOCAL_DEV_MSG_TRAFFIC_LIGHT_SET_KEY, is_green);
            traffic_light_set(is_green);
        }
        else if (LOCAL_DEV_MSG_GET_VIDEO_RECORD == msg_type)
        {
            auto start_date = msg(LOCAL_DEV_MSG_GET_VIDEO_RECORD_KEY1);
            auto end_date = msg(LOCAL_DEV_MSG_GET_VIDEO_RECORD_KEY2);
            auto video_path = get_video_record(start_date, end_date);
            neb::CJsonObject event_content;
            event_content.Add("msg_type", LOCAL_DEV_EVENT_GET_VIDEO_RECORD);
            event_content.Add(LOCAL_DEV_EVENT_GET_VIDEO_RECORD_KEY, video_path);
            send_event(event_content.ToString());
        }
        else if (LOCAL_DEV_MSG_GATE_CONTROL == msg_type)
        {
            bool is_close = false;
            msg.Get(LOCAL_DEV_MSG_GATE_CONTROL_KEY, is_close);
            gate_control(is_close);
        }
        else if (LOCAL_DEV_MSG_GATE_IS_CLOSE == msg_type)
        {
            auto is_close = gate_is_close();
            neb::CJsonObject event_content;
            event_content.Add("msg_type", LOCAL_DEV_EVENT_GATE_IS_CLOSE);
            event_content.Add(LOCAL_DEV_EVENT_GATE_IS_CLOSE_KEY, is_close, is_close);
            send_event(event_content.ToString());
        }
        else if (LOCAL_DEV_MSG_PRINTER_PRINT == msg_type)
        {
            auto content = msg(LOCAL_DEV_MSG_PRINTER_PRINT_KEY1);
            auto qr_code = msg(LOCAL_DEV_MSG_PRINTER_PRINT_KEY2);
            printer_print(content, qr_code);
        }
        else if (LOCAL_DEV_MSG_SCALE_CUR_WEIGHT == msg_type)
        {
            auto weight = scale_cur_weight();
            neb::CJsonObject event_content;
            event_content.Add("msg_type", LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT);
            event_content.Add(LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT_KEY, weight);
            send_event(event_content.ToString());
        }
    }
}