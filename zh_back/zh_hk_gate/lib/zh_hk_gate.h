#if !defined(_ZH_HK_GATE_H_)
#define _ZH_HK_GATE_H_

#include "HttpUtil.h"
#include "../../zh_tdf/tdf_include.h"
// #include "DataType.h"
// #include "DecodeCardSdk.h"
#include "HCNetSDK.h"
// #include "plaympeg4.h"
#include <functional>

enum zh_hk_gate_control_cmd {
    zh_hk_gate_close = 0,
    zh_hk_gate_open =1,
    zh_hk_gate_hold = 3,
};
struct hk_sub_callback_cfg {
    void *pData = nullptr;
    void (*callback)(const std::string &, const std::string &, void *);
};
void zh_hk_subcribe_event(const std::string &_road_ip, hk_sub_callback_cfg _callback);
void zh_hk_unsubcribe_event(const std::string &_road_ip);
void zh_hk_manual_trigger(const std::string &_road_ip);
void zh_hk_clear_event();
bool zh_hk_ctrl_gate(const std::string &_road_ip, zh_hk_gate_control_cmd _cmd);
bool zh_hk_ctrl_led(const std::string &_led_ip, const std::string &_content);
bool zh_hk_ctrl_voice(const std::string &_led_ip, const std::string &_content);


#endif // _ZH_HK_GATE_H_

