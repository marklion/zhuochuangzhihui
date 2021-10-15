#include "zh_hk_gate.h"
#include "../../zh_database/CJsonObject.hpp"

static tdf_log g_log("hk_gate");

bool zh_hk_ctrl_gate(const std::string &_gate_code, zh_hk_gate_control_cmd _cmd)
{
    bool ret = false;
    neb::CJsonObject req;
    req.Add("roadwaySyscode", _gate_code);
    req.Add("command", _cmd);

    int ret_len = 0;
    auto hk_ret = httpUtil::HTTPUTIL_Post("https://192.168.2.110/artemis/api/pms/v1/deviceControl", req.ToString().c_str(), getenv("HK_KEY"), getenv("HK_SEC"), 10, &ret_len);

    auto http_ret = httpUtil::HTTPUTIL_GetLastStatus();
    if (httpUtil::HTTPUTIL_ERR_SUCCESS == http_ret)
    {
        if (hk_ret)
        {
            neb::CJsonObject gate_ctrl_ret(std::string(hk_ret, ret_len));
            if (gate_ctrl_ret("code") == "0")
            {
                ret = true;
            }
            else
            {
                g_log.err("control gate err, code:%s, msg:%s, data:%s", gate_ctrl_ret("code").c_str(), gate_ctrl_ret("msg").c_str(), gate_ctrl_ret("data").c_str());
            }
            httpUtil::HTTPUTIL_Free(hk_ret);
        }
        else
        {
            g_log.err("no json resp get");
        }
    }
    else
    {
        g_log.err("http util failed:%ld", http_ret);
    }

    return ret;
}