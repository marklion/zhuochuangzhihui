#include "zh_hk_gate.h"
#include "../../zh_database/CJsonObject.hpp"
#include <vector>

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

struct hk_device_user_handler
{
    LONG user_id = -1;
    LONG al_handler = -1;
    hk_sub_callback_cfg callback_cfg;
};
static std::map<std::string, hk_device_user_handler> g_device_user_map;

void zh_hk_subcribe_event(const std::string &_road_ip, hk_sub_callback_cfg _callback_cfg)
{
    if (_road_ip.length() > 0)
    {
        NET_DVR_DEVICEINFO_V30 tmp_info = {0};
        auto user_id = NET_DVR_Login_V30(_road_ip.c_str(), 8000, "admin", "P@ssw0rd", &tmp_info);
        if (user_id >= 0)
        {
            NET_DVR_SetDVRMessageCallBack_V31(
                [](LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void *pUser) -> BOOL
                {
                    std::string plate_no;
                    std::string cam_ip(pAlarmer->sDeviceIP);
                    switch (lCommand)
                    {
                    case COMM_UPLOAD_PLATE_RESULT:
                    {
                        NET_DVR_PLATE_RESULT struPlateResult = {0};
                        memcpy(&struPlateResult, pAlarmInfo, sizeof(struPlateResult));
                        g_log.log("车牌号: %s\n", struPlateResult.struPlateInfo.sLicense); //车牌号
                        plate_no = std::string(struPlateResult.struPlateInfo.sLicense);
                        break;
                    }
                    case COMM_ITS_PLATE_RESULT:
                    {
                        NET_ITS_PLATE_RESULT struITSPlateResult = {0};
                        memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));
                        g_log.log("车牌号: %s\n", struITSPlateResult.struPlateInfo.sLicense); //车牌号
                        plate_no = std::string(struITSPlateResult.struPlateInfo.sLicense);
                        break;
                    }
                    default:
                        break;
                    }
                    auto hk_duh = g_device_user_map[cam_ip];
                    if (hk_duh.callback_cfg.callback)
                    {
                        hk_duh.callback_cfg.callback(plate_no, cam_ip, hk_duh.callback_cfg.pData);
                    }
                    return TRUE;
                },
                nullptr);
            NET_DVR_SETUPALARM_PARAM struSetupParam = {0};
            struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);
            struSetupParam.byLevel = 1;         //布防优先级：0- 一等级（高），1- 二等级（中）
            struSetupParam.byAlarmInfoType = 1; //上传报警信息类型: 0- 老报警信息(NET_DVR_PLATE_RESULT), 1- 新报警信息(NET_ITS_PLATE_RESULT)
            auto al_handler = NET_DVR_SetupAlarmChan_V41(user_id, &struSetupParam);
            if (al_handler != -1)
            {
                g_log.log("布防成功");
                hk_device_user_handler tmp;
                tmp.user_id = user_id;
                tmp.al_handler = al_handler;
                tmp.callback_cfg = _callback_cfg;
                g_device_user_map[_road_ip] = tmp;
            }
            else
            {
                g_log.err("NET_DVR_SetupAlarmChan_V41 failed, error code: %d\n", NET_DVR_GetLastError());
            }
        }
        else
        {
            g_log.err("failed to LOGIN device:%d", NET_DVR_GetLastError());
        }
    }
}

void zh_hk_unsubcribe_event(const std::string &_road_ip)
{
    auto hk_duh = g_device_user_map[_road_ip];
    if (hk_duh.al_handler != -1)
    {
        NET_DVR_CloseAlarmChan_V30(hk_duh.al_handler);
    }
    if (hk_duh.user_id != -1)
    {
        NET_DVR_Logout_V30(hk_duh.user_id);
    }
    g_device_user_map.erase(_road_ip);
}

void zh_hk_clear_event()
{
    for (auto itr = g_device_user_map.begin(); itr != g_device_user_map.end(); ++itr)
    {
        auto hk_duh = itr->second;
        if (hk_duh.al_handler != -1)
        {
            NET_DVR_CloseAlarmChan_V30(hk_duh.al_handler);
        }
        if (hk_duh.user_id != -1)
        {
            NET_DVR_Logout_V30(hk_duh.user_id);
        }
    }
    g_device_user_map.clear();
}

bool zh_hk_ctrl_gate(const std::string &_road_ip, zh_hk_gate_control_cmd _cmd)
{
    bool ret = false;

    auto hk_duh = g_device_user_map[_road_ip];
    if (hk_duh.user_id != -1)
    {
        NET_DVR_BARRIERGATE_CFG tmp = {0};
        tmp.dwSize = sizeof(tmp);
        tmp.dwChannel = 0;
        tmp.byLaneNo = 1;
        tmp.byBarrierGateCtrl = _cmd;
        ret = NET_DVR_RemoteControl(hk_duh.user_id, NET_DVR_BARRIERGATE_CTRL, &tmp, sizeof(tmp));
        if (ret)
        {
            g_log.log("open door %s", _road_ip.c_str());
        }
        else
        {
            g_log.err("open door %s, error_code:%d", _road_ip.c_str(), NET_DVR_GetLastError());
        }
    }

    return ret;
}

bool zh_hk_ctrl_led(const std::string &_led_ip, const std::string &_content)
{
    bool ret = false;
    NET_DVR_DEVICEINFO_V30 device_info;
    auto user_id = NET_DVR_Login_V30(_led_ip.c_str(), 8000, "admin", "12345", &device_info);
    if (user_id > 0)
    {
        std::vector<std::string> each_line;
        auto begin_pos = 0;
        for (auto i = 0; i < _content.size(); i++)
        {
            if (_content[i] == '\n')
            {
                each_line.push_back(_content.substr(begin_pos, i - begin_pos));
                begin_pos = i;
            }
        }

        DWORD channel_no = 0;
        for (auto &itr : each_line)
        {
            NET_DVR_LEDDISPLAY_CFG tmp_led_config = {0};
            tmp_led_config.dwSize = sizeof(tmp_led_config);
            strncpy(tmp_led_config.sDisplayInfo, itr.c_str(), sizeof(tmp_led_config.sDisplayInfo) - 1);
            tmp_led_config.byDisplayMode = 0;
            tmp_led_config.bySpeedType = 1;
            tmp_led_config.dwShowTime = 60;

            NET_DVR_STD_CONFIG tmp_std_config = {0};
            tmp_std_config.lpCondBuffer = &channel_no;
            tmp_std_config.dwCondSize = sizeof(channel_no);
            tmp_std_config.lpInBuffer = &tmp_led_config;
            tmp_std_config.dwInSize = sizeof(tmp_led_config);
            tmp_std_config.byDataType = 0;
            ret = NET_DVR_SetSTDConfig(user_id, NET_DVR_SET_LEDDISPLAY_CFG, &tmp_std_config);
            if (!ret)
            {
                g_log.err("failed to set led:%s, error_code:%d", _led_ip.c_str(), NET_DVR_GetLastError());
                break;
            }
            channel_no++;
        }
        if (ret)
        {
            g_log.log("set led:%s, content:%s", _led_ip.c_str(), _content.c_str());
        }
        NET_DVR_Logout(user_id);
    }
    else
    {
        g_log.err("failed to login led:%s, error_code:%d", _led_ip.c_str(), NET_DVR_GetLastError());
    }

    return ret;
}
bool zh_hk_ctrl_voice(const std::string &_led_ip, const std::string &_content)
{
    bool ret = false;
    NET_DVR_DEVICEINFO_V30 device_info;
    auto user_id = NET_DVR_Login_V30(_led_ip.c_str(), 8000, "admin", "12345", &device_info);
    if (user_id > 0)
    {
        DWORD channel_no = 0;
        NET_DVR_VOICEBROADCAST_CFG tmp_voice_config = {0};
        tmp_voice_config.dwSize = sizeof(tmp_voice_config);
        strncpy(tmp_voice_config.sInfo, _content.c_str(), sizeof(tmp_voice_config.sInfo) - 1);

        NET_DVR_STD_CONFIG tmp_std_config = {0};
        tmp_std_config.lpCondBuffer = &channel_no;
        tmp_std_config.dwCondSize = sizeof(channel_no);
        tmp_std_config.lpInBuffer = &tmp_voice_config;
        tmp_std_config.dwInSize = sizeof(tmp_voice_config);
        tmp_std_config.byDataType = 0;
        ret = NET_DVR_SetSTDConfig(user_id, NET_DVR_SET_LEDDISPLAY_CFG, &tmp_std_config);
        if (!ret)
        {
            g_log.err("failed to set voice:%s, error_code:%d", _led_ip.c_str(), NET_DVR_GetLastError());
        }
        channel_no++;
        if (ret)
        {
            g_log.log("set voice:%s, content:%s", _led_ip.c_str(), _content.c_str());
        }
        NET_DVR_Logout(user_id);
    }
    else
    {
        g_log.err("failed to login led:%s, error_code:%d", _led_ip.c_str(), NET_DVR_GetLastError());
    }
    return ret;
}

void __attribute__((constructor)) zh_hk_init(void)
{
    neb::CJsonObject req;
    req.AddEmptySubArray("eventTypes");
    req["eventTypes"].Add(771760130);
    req["eventTypes"].Add(771760133);
    req.Add("eventDest", "http://192.168.2.105/zh_rest/vehicle_event");
    call_hk_post("/api/eventService/v1/eventSubscriptionByEventTypes", req);
    NET_DVR_Init();
}
void __attribute__((destructor)) zh_hk_fini(void)
{
    NET_DVR_Cleanup();
}
void zh_hk_manual_trigger(const std::string &_road_ip)
{
    auto hk_duh = g_device_user_map[_road_ip];
    if (hk_duh.user_id != -1)
    {
        NET_DVR_SNAPCFG tmp = {0};
        tmp.dwSize = sizeof(tmp);
        tmp.bySnapTimes = 3;
        tmp.wSnapWaitTime = 1000;
        tmp.wIntervalTime[0] = 1000;
        tmp.wIntervalTime[1] = 1000;
        tmp.byRelatedDriveWay = 0;
        if (NET_DVR_ContinuousShoot(hk_duh.user_id, &tmp))
        {
            g_log.log("trigger cap:%s", _road_ip.c_str());
        }
        else
        {
            g_log.err("failed to trigger cap:%s, error_code:%d", _road_ip.c_str(), NET_DVR_GetLastError());
        }
    }
}