#include "../abs_device_driver.h"

class fake_cam_driver : public abs_device_driver, public epoll_node_t
{
    std::string get_static_config_file(const std::string &_key)
    {
        std::string ret;
        auto config_obj = zh_plugin_conf_get_config(get_device_arg("file").c_str());
        ret = config_obj(_key);
        return ret;
    }
    void show_actions(const std::string &_action_string)
    {
        tdf_log action_log("fake_dev_log", get_device_arg("log"), "");
        action_log.log(_action_string);
    }
public:
    bool cur_gate_is_close = false;
    std::string pic_path;
    using abs_device_driver::abs_device_driver;
    // 车牌相机能力
    virtual std::string take_picture()
    {
        return pic_path;
    }
    virtual void manual_trigger()
    {
        show_actions("手动触发");
    }

    // 屏幕能力
    virtual void led_show(const std::string &_l1, const std::string &_l2, const std::string &_l3, const std::string &_l4)
    {
        show_actions("屏幕显示：\n" + _l1 + "\n" + _l2 + "\n" + _l3 + "\n" + _l4);
    }

    // 喇叭能力
    virtual void speaker_speak(const std::string &_voice_content) {
        show_actions("喇叭播报:" + _voice_content);
    }

    // 红绿灯能力
    virtual void traffic_light_set(bool _is_green) {
show_actions("红绿灯:" + std::to_string(_is_green));
    }

    // 监控能力
    virtual std::string get_video_record(const std::string &_begin_date, const std::string &_end_date)
    {
        return get_static_config_file("record");
    }

    // 道闸能力
    virtual void gate_control(bool _make_close) {
        cur_gate_is_close = _make_close;
    }
    virtual bool gate_is_close()
    {
        return cur_gate_is_close;
    }

    // 打印机能力
    virtual void printer_print(const std::string &_content, const std::string &_qr_code) {
        show_actions("打印内容：" + _content + "二维码:" + _qr_code);
    }

    // 地磅能力
    virtual double scale_cur_weight()
    {
        return std::stod(get_static_config_file("weight"));
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

int main(int argc, char const *argv[])
{
    fake_cam_driver fcd(argc, argv, {"file", "log"});

    while (1)
    {
        fcd.proc_msg();
    }
    sleep(15);

    return 0;
}
