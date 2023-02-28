#if !defined(_ABS_DEVICE_DRIVER_H_)
#define _ABS_DEVICE_DRIVER_H_

#include "../utils/local_utils.h"

class abs_device_driver {
public:
    abs_device_driver(int argc, const char *const* argv, const std::vector<std::string> &_device_arg_param);
    virtual ~abs_device_driver();
    int get_fd() const;
    std::string get_device_arg(const std::string &_arg_key) const;
    void proc_msg();
    void send_event(const std::string &_event);
    // 车牌识别事件
    void pub_vehicle_come(const std::string &_vehicle_number);

    // 身份证事件
    void pub_id_come(const std::string &_id_no);

    // 二维码事件
    void pub_qr_scan(const std::string &_qr_code);

    void pub_device_status(bool _is_healthy);
protected:
    // 车牌相机能力
    virtual std::string take_picture()
    {
        return "";
    }
    virtual void manual_trigger() {}

    // 屏幕能力
    virtual void led_show(const std::string &_l1, const std::string &_l2, const std::string &_l3, const std::string &_l4) {}

    // 喇叭能力
    virtual void speaker_speak(const std::string &_voice_content) {}

    // 红绿灯能力
    virtual void traffic_light_set(bool _is_green) {}

    // 监控能力
    virtual std::string get_video_record(const std::string &_begin_date, const std::string &_end_date)
    {
        return "";
    }

    // 道闸能力
    virtual void gate_control(bool _make_close) {}
    virtual bool gate_is_close()
    {
        return false;
    }

    // 打印机能力
    virtual void printer_print(const std::string &_content, const std::string &_qr_code) {}

    // 地磅能力
    virtual double scale_cur_weight()
    {
        return 0;
    }

    int fd = -1;
    std::string center_name;
    std::string self_name;
    std::map<std::string, std::string> device_args;
};

#endif // _ABS_DEVICE_DRIVER_H_
