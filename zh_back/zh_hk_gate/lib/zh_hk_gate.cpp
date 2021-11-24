#include "zh_hk_gate.h"
#include "../../zh_database/CJsonObject.hpp"

static tdf_log g_log("hk_gate");

static neb::CJsonObject call_hk_post(const std::string &_url, neb::CJsonObject req)
{
    int ret_len = 0;
    neb::CJsonObject ret;
    g_log.log("call api:%s param:%s", _url.c_str(), req.ToString().c_str());
    std::string url = "https://192.168.2.110/artemis" + _url;
    auto hk_ret = httpUtil::HTTPUTIL_Post(url.c_str(), req.ToString().c_str(), getenv("HK_KEY"), getenv("HK_SEC"), 10, &ret_len);
    auto http_ret = httpUtil::HTTPUTIL_GetLastStatus();
    if (httpUtil::HTTPUTIL_ERR_SUCCESS == http_ret)
    {
        if (hk_ret)
        {
            neb::CJsonObject gate_ctrl_ret(std::string(hk_ret, ret_len));
            if (gate_ctrl_ret("code") == "0")
            {
                ret = gate_ctrl_ret;
            }
            else
            {
                g_log.err("control gate err, code:%s, msg:%s, data:%s", gate_ctrl_ret("code").c_str(), gate_ctrl_ret("msg").c_str(), gate_ctrl_ret("data").c_str());
            }
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
    if (nullptr != hk_ret)
    {
        httpUtil::HTTPUTIL_Free(hk_ret);
    }

    return ret;
}

void zh_hk_subcribe_event()
{
    neb::CJsonObject req;
    req.AddEmptySubArray("eventTypes");
    req["eventTypes"].Add(771760130);
    req["eventTypes"].Add(771760133);
    req.Add("eventDest", "http://192.168.2.105/zh_rest/vehicle_event");
    call_hk_post("/api/eventService/v1/eventSubscriptionByEventTypes", req);
}
bool zh_hk_ctrl_gate(const std::string &_gate_code, zh_hk_gate_control_cmd _cmd)
{
    bool ret = false;
    neb::CJsonObject req;
    req.Add("roadwaySyscode", _gate_code);
    req.Add("command", _cmd);

    auto hk_ret = call_hk_post("/api/pms/v1/deviceControl", req);
    if (hk_ret("code") == "0")
    {
        ret = true;
    }

    return ret;
}

bool zh_hk_ctrl_led(const std::string &_gate_code, const std::string &_content)
{
    bool ret = false;
    neb::CJsonObject req;
    req.Add("roadwaySyscode", _gate_code);
    req.AddEmptySubArray("ledContent");
    std::list<std::string> each_line;
    int start = 0;
    int end = 0;
    std::string tmp = _content;
    tmp.push_back('\n');

    while (end < tmp.length())
    {
        if (tmp[end] == '\n')
        {
            each_line.push_back(tmp.substr(start, end - start));
            start = end + 1;
        }
        end++;
    }

    int line_number = 1;
    for (auto &itr:each_line)
    {
        neb::CJsonObject tmp;
        tmp.Add("line", line_number++);
        tmp.Add("fontConfig", "[1,1,1]");
        tmp.Add("showConfig", itr);
        req["ledContent"].Add(tmp);
    }

    auto hk_ret = call_hk_post("/api/pms/v1/device/led/control", req);
    if (hk_ret("code") == "0")
    {
        ret = true;
    }

    return ret;
}
bool zh_hk_ctrl_voice(const std::string &_gate_code, const std::string &_content)
{
    bool ret = false;
    neb::CJsonObject req;
    req.Add("roadwaySyscode", _gate_code);
    req.Add("voiceContent", _content);

    auto hk_ret = call_hk_post("/api/pms/v1/device/voice/control", req);
    if (hk_ret("code") == "0")
    {
        ret = true;
    }

    return ret;
}