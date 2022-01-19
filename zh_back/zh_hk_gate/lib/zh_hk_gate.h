#if !defined(_ZH_HK_GATE_H_)
#define _ZH_HK_GATE_H_

#include "HttpUtil.h"
#include "../../zh_tdf/tdf_include.h"
// #include "DataType.h"
// #include "DecodeCardSdk.h"
#include "HCNetSDK.h"
// #include "plaympeg4.h"
#include <functional>
#include "../../zh_rpc_server/zh_rpc_util.h"

enum zh_hk_gate_control_cmd {
    zh_hk_gate_close = 0,
    zh_hk_gate_open =1,
    zh_hk_gate_hold = 3,
};

enum zh_hk_led_type {
    zh_hk_led_enter_gate = 0,
    zh_hk_led_exit_gate,
    zh_hk_led_cannot_exit_gate,
    zh_hk_led_enter_scale,
    zh_hk_led_cannot_enter_scale,
    zh_hk_led_exit_scale,
};

bool zh_hk_subcribe_event(const std::string &_road_ip, zh_sub_callback_cfg _callback);
void zh_hk_unsubcribe_event(const std::string &_road_ip);
void zh_hk_manual_trigger(const std::string &_road_ip);
void zh_hk_clear_event();
bool zh_hk_ctrl_gate(const std::string &_road_ip, zh_hk_gate_control_cmd _cmd);
bool zh_hk_ctrl_led(const std::string &_led_ip, const std::string &_content);
bool zh_hk_ctrl_led(const std::string &_led_ip, zh_hk_led_type content_type, const std::string &_vehicle_number);
bool zh_hk_ctrl_voice(const std::string &_led_ip, const std::string &_content);
bool zh_hk_cast_empty(const std::string &_led_ip);
bool zh_hk_cast_enter_scale(const std::string &_led_ip, const std::string &_plate_no);
bool zh_hk_cast_no_order(const std::string &_led_ip, const std::string &_plate_no);
bool zh_hk_cast_no_call(const std::string &_led_ip, const std::string &_plate_no);
bool zh_hk_cast_no_confirm(const std::string &_led_ip, const std::string &_plate_no);
bool zh_hk_cast_holding(const std::string &_led_ip);
bool zh_hk_cast_exit_scale(const std::string &_led_ip, const std::string &_weight);
bool zh_hk_cast_exit_busy(const std::string &_led_ip);
bool zh_hk_cast_welcome(const std::string &_led_ip, const std::string &_plate_no);
bool zh_hk_cast_cannot_leave(const std::string &_led_ip, const std::string &_plate_no);
bool zh_hk_cast_leave_bye(const std::string &_led_ip, const std::string &_plate_no);
std::string zh_hk_get_channel_video(const std::string &_nvr_ip, int _channel_id, const NET_DVR_TIME &_start, const NET_DVR_TIME &_end);


#endif // _ZH_HK_GATE_H_

