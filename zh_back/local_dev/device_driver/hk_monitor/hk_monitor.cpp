#include "zh_hk_gate.h"
#include "../abs_device_driver.h"
static NET_DVR_TIME strTime2unix(const std::string &timeStamp)
{
    NET_DVR_TIME tm;
    memset(&tm, 0, sizeof(tm));

    sscanf(
        timeStamp.c_str(), "%d-%d-%d %d:%d:%d",
        &tm.dwYear, &tm.dwMonth, &tm.dwDay,
        &tm.dwHour, &tm.dwMinute, &tm.dwSecond);

    return tm;
}
class hk_monitor : public abs_device_driver
{
public:
    using abs_device_driver::abs_device_driver;
    void init_self() {
        zh_sub_callback_cfg tmp;
        tmp.callback = [](const std::string &_plate, const std::string &, void *_private){
            auto p_this = (hk_monitor *)_private;
            p_this->pub_vehicle_come(_plate);
        };
        tmp.pData = this;
        zh_hk_subcribe_event(get_device_arg("ip"), tmp);
    }

    virtual std::string get_video_record(const std::string &_begin_date, const std::string &_end_date)
    {
        return zh_hk_get_channel_video(get_device_arg("ip"), atoi(get_device_arg("channel").c_str()), strTime2unix(_begin_date), strTime2unix(_end_date), get_device_arg("username"), get_device_arg("password"));
    }
// 车牌相机能力
    virtual std::string take_picture()
    {
        return zh_hk_get_capture_picture(get_device_arg("ip"), atoi(get_device_arg("channel").c_str()), get_device_arg("username"), get_device_arg("password"));
    }
    virtual void manual_trigger()
    {
        zh_hk_manual_trigger(get_device_arg("ip"));
    }

    // 屏幕能力
    virtual void led_show(const std::string &_l1, const std::string &_l2, const std::string &_l3, const std::string &_l4)
    {
        async_led_post(get_device_arg("led_ip"), hk_led_make_cmd(_l3, _l4));
    }

    // 喇叭能力
    virtual void speaker_speak(const std::string &_voice_content)
    {
    }

    // 道闸能力
    virtual void gate_control(bool _make_close)
    {
        auto cmd = zh_hk_gate_control_cmd::zh_hk_gate_open;
        if (_make_close)
        {
            cmd = zh_hk_gate_control_cmd::zh_hk_gate_close;
        }
        zh_hk_ctrl_gate(get_device_arg("ip"), cmd);
    }
    virtual bool gate_is_close()
    {
        return zh_hk_get_cam_IO(get_device_arg("ip"));
    }
};

int main(int argc, char const *argv[])
{
    hk_monitor hm(argc, argv, {"ip", "channel", "username", "password", "led_ip"});
    hm.init_self();
    while (1)
    {
        hm.proc_msg();
        hm.pub_device_status(true);
    }
    hm.pub_device_status(false);
    sleep(15);

    return 0;
}
