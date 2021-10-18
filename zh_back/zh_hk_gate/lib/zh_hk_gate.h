#if !defined(_ZH_HK_GATE_H_)
#define _ZH_HK_GATE_H_

#include "HttpUtil.h"
#include "../../zh_tdf/tdf_include.h"

enum zh_hk_gate_control_cmd {
    zh_hk_gate_close = 0,
    zh_hk_gate_open =1,
    zh_hk_gate_hold = 3,
};

bool zh_hk_ctrl_gate(const std::string &_gate_code, zh_hk_gate_control_cmd _cmd);
bool zh_hk_ctrl_led(const std::string &_gate_code, const std::string &_content);
bool zh_hk_ctrl_voice(const std::string &_gate_code, const std::string &_content);

#endif // _ZH_HK_GATE_H_

