#include "../abs_device_driver.h"

class fake_cam_driver : public abs_device_driver, public epoll_node_t
{
    std::string send_and_recv_ask_ui(const std::string &_cmd, bool no_need_resp = false)
    {
        std::string ret;

        auto fd = connect_to_device_tcp_server(get_device_arg("ip"), (unsigned short)(atoi(get_device_arg("port").c_str())));
        send(fd, _cmd.data(), _cmd.length(), 0);
        char buff[4096] = "";
        if (!no_need_resp)
        {
            auto recv_len = recv(fd, buff, sizeof(buff), 0);
            if (recv_len > 0)
            {
                ret.assign(buff, recv_len);
            }
        }
        close(fd);

        return ret;
    }

public:
    bool cur_gate_is_close = false;
    double cur_weight = 0;
    using abs_device_driver::abs_device_driver;
    // 车牌相机能力
    virtual std::string take_picture()
    {
        return send_and_recv_ask_ui("拍照车牌");
    }
    virtual void manual_trigger()
    {
       send_and_recv_ask_ui("手动触发", true);
    }

    // 屏幕能力
    virtual void led_show(const std::string &_l1, const std::string &_l2, const std::string &_l3, const std::string &_l4)
    {
        send_and_recv_ask_ui("屏幕显示：\n" + _l1 + "\n" + _l2 + "\n" + _l3 + "\n" + _l4, true);
    }

    // 喇叭能力
    virtual void speaker_speak(const std::string &_voice_content) {
        send_and_recv_ask_ui("喇叭播报:" + _voice_content, true);
    }

    // 红绿灯能力
    virtual void traffic_light_set(bool _is_green) {
        send_and_recv_ask_ui("红绿灯:" + std::to_string(_is_green), true);
    }

    // 监控能力
    virtual std::string get_video_record(const std::string &_begin_date, const std::string &_end_date)
    {
        return send_and_recv_ask_ui("获取录像:" + _begin_date + "到" + _end_date);
    }

    // 道闸能力
    virtual void gate_control(bool _make_close) {
        send_and_recv_ask_ui("道闸控制：" + std::to_string(_make_close), true);
    }
    virtual bool gate_is_close()
    {
        return cur_gate_is_close;
    }

    // 打印机能力
    virtual void printer_print(const std::string &_content, const std::string &_qr_code) {
        send_and_recv_ask_ui("打印内容：" + _content + "二维码:" + _qr_code, true);
    }

    // 地磅能力
    virtual double scale_cur_weight()
    {
        return cur_weight;
    }

    virtual bool proc_in()
    {
        proc_msg();
        return true;
    }
    virtual int get_fd() const
    {
        return abs_device_driver::get_fd();
    }
};

class fake_device_ui : public epoll_node_t
{
public:
    int fd = -1;
    fake_cam_driver *m_dev = nullptr;
    virtual bool proc_in()
    {
        char buff[2048] = {0};
        recv(fd, buff, sizeof(buff), 0);
        switch (buff[0])
        {
        case 'v':
            m_dev->pub_vehicle_come(&buff[1]);
            break;
        case 'i':
            m_dev->pub_id_come(&buff[1]);
            break;
        case 'q':
            m_dev->pub_qr_scan(&buff[1]);
            break;
        case 'g':
            m_dev->cur_gate_is_close = true;
            break;
        case 'G':
            m_dev->cur_gate_is_close = false;
            break;
        case 'w':
            m_dev->cur_weight = std::stod(&buff[1]);
            break;
        default:
            break;
        }
        return true;
    }
    virtual int get_fd() const
    {
        return fd;
    }
};

int main(int argc, char const *argv[])
{
    fake_cam_driver fcd(argc, argv, {"ip", "port"});
    fake_device_ui fdui;
    fdui.fd = connect_to_device_tcp_server(fcd.get_device_arg("ip"), (unsigned short)(atoi(fcd.get_device_arg("port").c_str())));
    fdui.m_dev = &fcd;
    epoll_sch_center sch;
    sch.add_node(fcd);
    sch.add_node(fdui);
    sch.run();

    return 0;
}
