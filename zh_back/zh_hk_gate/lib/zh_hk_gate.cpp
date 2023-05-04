#include "zh_hk_gate.h"
#include "../../zh_database/zh_db_config.h"
#include <vector>
#include <iconv.h>

static tdf_log g_log("hk_gate");

int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}
//UNICODE码转为GB2312码
int u2g(char *inbuf, int inlen, char *outbuf, int outlen)
{
    return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}
//GB2312码转为UNICODE码
int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}

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

std::string gbk2utf(const std::string &_gbk)
{
    char in_buff[2048] = {0};
    char out_buff[2048] = {0};
    strcpy(in_buff, _gbk.c_str());
    g2u(in_buff, strlen(in_buff), out_buff, sizeof(out_buff));
    return std::string(out_buff);
}
std::string utf2gbk(const std::string &_gbk)
{
    char in_buff[2048] = {0};
    char out_buff[2048] = {0};
    strcpy(in_buff, _gbk.c_str());
    u2g(in_buff, strlen(in_buff), out_buff, sizeof(out_buff));
    return std::string(out_buff);
}

struct hk_device_user_handler
{
    LONG user_id = -1;
    LONG al_handler = -1;
    zh_sub_callback_cfg callback_cfg;
};
static std::map<std::string, hk_device_user_handler> g_device_user_map;

struct hk_device_timer_param {
    zh_sub_callback_cfg callback_cfg;
    std::string road_ip;
};
void MessageCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void *pUser)
{
    int i;
    NET_ITC_STATUS_DETECT_RESULT struAlarmInfo; //状态监测结果结构体
    memcpy(&struAlarmInfo, pAlarmInfo, sizeof(NET_ITC_STATUS_DETECT_RESULT));
    switch (lCommand)
    {
    case COMM_ITS_GATE_ALARMINFO:
    {
        NET_DVR_GATE_ALARMINFO al_info;
        memcpy(&al_info, pAlarmInfo, sizeof(al_info));
        break;
    }
    case COMM_ITC_STATUS_DETECT_RESULT:
    {
        switch (struAlarmInfo.dwStatusType)
        {
        case 1: // IO 触发
        {
            auto hk_duh = g_device_user_map[pAlarmer->sDeviceIP];
            if (struAlarmInfo.uStatusParam.struTrigIO.byTriggerIOIndex[1] == 1)
            {
                hk_duh.callback_cfg.is_close = true;
                g_log.log("door is close");
            }
            if (struAlarmInfo.uStatusParam.struTrigIO.byTriggerIOIndex[2] == 1)
            {
                hk_duh.callback_cfg.is_close = false;
                g_log.log("door is open");
            }
            break;
        }
        default:
            break;
        }
    }
    break;
    default:
        break;
    }
}
bool zh_hk_subcribe_event(const std::string &_road_ip, zh_sub_callback_cfg _callback_cfg)
{
    bool ret = false;
    if (_road_ip.length() > 0)
    {
        NET_DVR_DEVICEINFO_V30 tmp_info = {0};
        auto user_id = NET_DVR_Login_V30(_road_ip.c_str(), 8000, "admin", "P@ssw0rd", &tmp_info);
        if (user_id >= 0)
        {
            NET_DVR_IO_INCFG io_cfg = {0};
            io_cfg.byIoInStatus = 0;
            io_cfg.dwSize = sizeof(io_cfg);
            if (!NET_DVR_SetDVRConfig(user_id, NET_DVR_SET_IOINCFG, 2, &io_cfg, sizeof(io_cfg)))
            {
                g_log.err("failed to set io cfg:%d", NET_DVR_GetLastError());
            }
            NET_DVR_SetDVRMessageCallBack_V31(
                [](LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void *pUser) -> BOOL
                {
                    int ev_type = 0;
                    bool is_close = false;
                    std::string plate_no;
                    std::string cam_ip(pAlarmer->sDeviceIP);
                    switch (lCommand)
                    {
                    case COMM_ITS_GATE_ALARMINFO:
                    {
                        ev_type = 1;
                        NET_DVR_GATE_ALARMINFO al_info;
                        memcpy(&al_info, pAlarmInfo, sizeof(al_info));
                        if (al_info.byExternalDevStatus == DEVICES_STATUS_CLOSED)
                        {
                            is_close = true;
                            g_log.log("door (%s) is close", cam_ip.c_str());
                        }
                        if (al_info.byExternalDevStatus == DEVICES_STATUS_OPENED)
                        {
                            is_close = false;
                            g_log.log("door (%s) is open", cam_ip.c_str());
                        }
                        break;
                    }
                    case COMM_UPLOAD_PLATE_RESULT:
                    {
                        ev_type = 2;
                        NET_DVR_PLATE_RESULT struPlateResult = {0};
                        memcpy(&struPlateResult, pAlarmInfo, sizeof(struPlateResult));
                        g_log.log("车牌号: %s\n", struPlateResult.struPlateInfo.sLicense); //车牌号
                        plate_no = std::string(struPlateResult.struPlateInfo.sLicense);
                        break;
                    }
                    case COMM_ITS_PLATE_RESULT:
                    {
                        ev_type = 2;
                        NET_ITS_PLATE_RESULT struITSPlateResult = {0};
                        memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));
                        g_log.log("车牌号: %s\n", struITSPlateResult.struPlateInfo.sLicense); //车牌号
                        plate_no = std::string(struITSPlateResult.struPlateInfo.sLicense);
                        break;
                    }
                    default:
                        break;
                    }
                    auto &hk_duh = g_device_user_map[cam_ip];
                    if (ev_type == 1)
                    {
                        hk_duh.callback_cfg.is_close = is_close;
                    }
                    else if (ev_type == 2)
                    {

                        if (plate_no.length() > 2)
                        {
                            plate_no = plate_no.substr(2, plate_no.length() - 2);
                        }
                        plate_no = gbk2utf(plate_no);
                        if (plate_no == "车牌")
                        {
                            g_log.log("cap no plate no");
                            return TRUE;
                        }
                        if (hk_duh.callback_cfg.callback)
                        {
                            hk_duh.callback_cfg.callback(plate_no, cam_ip, hk_duh.callback_cfg.pData);
                        }
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
                ret = true;
                zh_runtime_get_device_health()[_road_ip] = 1;
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
    if (!ret)
    {
        zh_runtime_get_device_health()[_road_ip] = 2;
        auto timer_param = new hk_device_timer_param();
        timer_param->road_ip = _road_ip;
        timer_param->callback_cfg = _callback_cfg;
        tdf_main::get_inst().start_timer(
            5,
            [](void *_private)
            {
                auto timer_param = (hk_device_timer_param *)(_private);
                zh_hk_subcribe_event(timer_param->road_ip, timer_param->callback_cfg);
                delete timer_param;
            },
            timer_param, true);
    }
    return ret;
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

struct hk_gate_ctrl_param
{
    std::string road_ip;
    zh_hk_gate_control_cmd cmd;
};

bool zh_hk_get_cam_IO(const std::string &_nvr_ip)
{
    bool ret = false;

    auto hk_duh = g_device_user_map[_nvr_ip];
    if (hk_duh.user_id != -1)
    {
        ret = hk_duh.callback_cfg.is_close;
    }

    return ret;
}
bool zh_hk_ctrl_gate(const std::string &_road_ip, zh_hk_gate_control_cmd _cmd)
{
    auto gate_cp = new hk_gate_ctrl_param();
    gate_cp->road_ip = _road_ip;
    gate_cp->cmd = _cmd;
    tdf_main::get_inst().Async_to_workthread(
        [](void *_private, const std::string &chrct)
        {
            auto gate_cp = (hk_gate_ctrl_param *)(_private);
            auto hk_duh = g_device_user_map[gate_cp->road_ip];
            if (hk_duh.user_id != -1)
            {
                NET_DVR_BARRIERGATE_CFG tmp = {0};
                tmp.dwSize = sizeof(tmp);
                tmp.dwChannel = 1;
                tmp.byLaneNo = 1;
                tmp.byBarrierGateCtrl = gate_cp->cmd;
                auto ret = NET_DVR_RemoteControl(hk_duh.user_id, NET_DVR_BARRIERGATE_CTRL, &tmp, sizeof(tmp));
                if (ret)
                {
                    g_log.log("open door %s", gate_cp->road_ip.c_str());
                    zh_runtime_get_device_health()[gate_cp->road_ip] = 1;
                }
                else
                {
                    g_log.err("open door %s, error_code:%d", gate_cp->road_ip.c_str(), NET_DVR_GetLastError());
                    zh_runtime_get_device_health()[gate_cp->road_ip] = 2;
                }
            }
            delete gate_cp;
        },
        gate_cp, "");
    return true;
}

struct hk_led_connector
{
    int socket_fd = -1;
    std::string m_ip;
    hk_led_connector(const std::string &_ip) : m_ip(_ip)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd >= 0)
        {
            sockaddr_in led_addr = {0};
            led_addr.sin_addr.s_addr = inet_addr(_ip.c_str());
            led_addr.sin_family = AF_INET;
            led_addr.sin_port = htons(10000);
            if (0 == connect(fd, (sockaddr *)&led_addr, sizeof(led_addr)))
            {
                socket_fd = fd;
            }
            else
            {
                g_log.err("failed to connect:%s", strerror(errno));
                close(fd);
            }
        }
        else
        {
            g_log.err("failed to socket:%s", strerror(errno));
        }
    }
    bool send_cmd(const std::string &_cmd)
    {
        bool ret = false;
        if (socket_fd >= 0)
        {
            g_log.log("send to led:");
            g_log.log_package(_cmd.data(), _cmd.size());
            if (_cmd.size() == send(socket_fd, _cmd.data(), _cmd.size(), 0))
            {
                char buff[1024];
                usleep(300000);
                auto recv_len = recv(socket_fd, buff, sizeof(buff), MSG_DONTWAIT);
                if (recv_len > 0)
                {
                    g_log.log("recv from led:");
                    g_log.log_package(buff, recv_len);
                }
                ret = true;
            }
        }
        if (ret)
        {
            zh_runtime_get_device_health()[m_ip] = 1;
        }
        else
        {
            zh_runtime_get_device_health()[m_ip] = 2;
        }

        return ret;
    }
    ~hk_led_connector()
    {
        if (socket_fd >= 0)
        {
            close(socket_fd);
        }
    }
};

void __attribute__((constructor)) zh_hk_init(void)
{
    // neb::CJsonObject req;
    // req.AddEmptySubArray("eventTypes");
    // req["eventTypes"].Add(771760130);
    // req["eventTypes"].Add(771760133);
    // req.Add("eventDest", "http://192.168.2.105/zh_rest/vehicle_event");
    // call_hk_post("/api/eventService/v1/eventSubscriptionByEventTypes", req);
    NET_DVR_Init();
    if (!NET_DVR_SetConnectTime(1000, 1) ||
        !NET_DVR_SetReconnect(10000, true) ||
        !NET_DVR_SetRecvTimeOut(1000))
    {
        g_log.err("failed to init hk_lib:%d", NET_DVR_GetLastError());
    }
}
void __attribute__((destructor)) zh_hk_fini(void)
{
    NET_DVR_Cleanup();
}
void zh_hk_manual_trigger(const std::string &_road_ip)
{
    g_log.log("manual trigger %s", _road_ip.c_str());
    auto hk_duh = g_device_user_map[_road_ip];
    if (hk_duh.user_id != -1)
    {
        NET_DVR_SNAPCFG tmp = {0};
        tmp.dwSize = sizeof(tmp);
        tmp.bySnapTimes = 3;
        tmp.wSnapWaitTime = 1;
        tmp.wIntervalTime[0] = 1000;
        tmp.wIntervalTime[1] = 1000;
        tmp.byRelatedDriveWay = 0;
        if (NET_DVR_ContinuousShoot(hk_duh.user_id, &tmp))
        {
            zh_runtime_get_device_health()[_road_ip] = 1;
            g_log.log("trigger cap:%s", _road_ip.c_str());
        }
        else
        {
            zh_runtime_get_device_health()[_road_ip] = 2;
            g_log.err("failed to trigger cap:%s, error_code:%d", _road_ip.c_str(), NET_DVR_GetLastError());
        }
    }
}

enum hk_cast_type
{
    empty,
    enter_scale,
    no_order,
    no_call,
    no_confirm,
    holding,
    exit_scale,
    busy,
    welcome,
    cannot_leave,
    leave_bye
};

static std::string file_type_map[] = {
    "empty", "enter_scale", "no_order", "no_call", "no_confirm", "holding", "exit_scale", "busy", "welcome", "cannot_leave", "leave_bye"

};

#define ZH_HK_ORIGINAL_FRAME(ret, frame) (ret).append((char *)(frame), sizeof(frame))
std::string hk_led_make_text_block(const std::string &_msg, char _pos, char _color, char _action = 0x01)
{
    std::string ret;
    unsigned char zone_no[] = {_pos + 1};
    int zone_len = 26;
    unsigned char zone_type[] = {0x0e};
    unsigned char zone_position[] = {0x00, 0x00, 16 * _pos, 0x00, 0x3f, 0x00, 0x0f + 16 * _pos, 0x00};
    unsigned char zone_color[] = {_color};
    unsigned char reserved[2] = {0};
    unsigned char zone_action[] = {_action, 0xff};
    unsigned char zone_speed[] = {15};
    unsigned char zone_stay[] = {3};
    unsigned char zone_font[] = {0x10};
    int string_len = 0;

    auto text_string = utf2gbk(_msg);
    string_len = text_string.size();
    zone_len += string_len;

    ZH_HK_ORIGINAL_FRAME(ret, zone_no);
    ret.append((char *)&zone_len, sizeof(zone_len));
    ZH_HK_ORIGINAL_FRAME(ret, zone_type);
    ZH_HK_ORIGINAL_FRAME(ret, zone_position);
    ZH_HK_ORIGINAL_FRAME(ret, zone_color);
    ZH_HK_ORIGINAL_FRAME(ret, reserved);
    ZH_HK_ORIGINAL_FRAME(ret, zone_action);
    ZH_HK_ORIGINAL_FRAME(ret, zone_speed);
    ZH_HK_ORIGINAL_FRAME(ret, zone_stay);
    ZH_HK_ORIGINAL_FRAME(ret, zone_font);
    ret.append((char *)&string_len, sizeof(string_len));
    ret.append(text_string);

    g_log.log("make oem_block");
    g_log.log_package(ret.data(), ret.length());
    return ret;
}
std::string hk_led_make_program_oem_data()
{
    std::string text_string(getenv("OEM_SHORT") ? getenv("OEM_SHORT") : getenv("OEM_NAME"));
    return hk_led_make_text_block(text_string, 0, 1);
}
std::string hk_led_make_program_time_data()
{
    std::string ret;
    auto time_string = zh_rpc_util_get_timestring();
    ret = hk_led_make_text_block(time_string, 1, 1, 0x20);
    g_log.log("make time_block");
    g_log.log_package(ret.data(), ret.length());
    return ret;
}
std::string hk_led_make_program_msg_data(const std::string &_msg)
{
    return hk_led_make_text_block(_msg, 2, 1, 0x20);
}
std::string hk_led_make_program_plate_data(const std::string &_plate_no)
{
    return hk_led_make_text_block(_plate_no, 3, 1);
}

std::string hk_led_make_program_voice_data(const std::string &_voice)
{
    std::string ret;
    unsigned char zone_no[] = {0x05};
    int zone_len = 26;
    unsigned char zone_type[] = {0x2D};
    unsigned char zone_reserved[15] = {0};
    unsigned char zone_circle[] = {0x01};
    int string_len = 0;

    auto voice_string = utf2gbk(_voice);
    string_len = voice_string.length();
    zone_len += voice_string.length();

    ZH_HK_ORIGINAL_FRAME(ret, zone_no);
    ret.append((char *)&zone_len, sizeof(zone_len));
    ZH_HK_ORIGINAL_FRAME(ret, zone_type);
    ZH_HK_ORIGINAL_FRAME(ret, zone_reserved);
    ZH_HK_ORIGINAL_FRAME(ret, zone_circle);
    ret.append((char *)&string_len, sizeof(string_len));
    ret.append(voice_string);

    g_log.log("make voice_block");
    g_log.log_package(ret.data(), ret.length());
    return ret;
}

std::string hk_led_make_frame_data(const std::string &_msg, const std::string &_plate_no)
{
    std::string ret;

    unsigned char program_count[] = {0x01};
    unsigned char program_no[] = {0x01};
    unsigned int program_len = 0;
    unsigned char zone_count[] = {0x01};
    unsigned char reserve[18] = {0};

    if (_msg.length() > 0)
    {
        zone_count[0] = 0x04;
        if (_plate_no.length() > 0)
        {
            zone_count[0]++;
        }
    }
    auto program_blocks = hk_led_make_program_oem_data();
    if (_msg.length() > 0)
    {
        program_blocks += hk_led_make_program_time_data();
    }
    if (_msg.length() > 0)
    {
        program_blocks += hk_led_make_program_msg_data(_msg);
        if (_plate_no.length() > 0)
        {
            program_blocks += hk_led_make_program_plate_data(_plate_no);
        }
        program_blocks += hk_led_make_program_voice_data(_msg + _plate_no);
    }
    program_len = program_blocks.length() + 24;

    ZH_HK_ORIGINAL_FRAME(ret, program_count);
    ZH_HK_ORIGINAL_FRAME(ret, program_no);
    ret.append((char *)&program_len, sizeof(program_len));
    ZH_HK_ORIGINAL_FRAME(ret, zone_count);
    ZH_HK_ORIGINAL_FRAME(ret, reserve);
    ret.append(program_blocks);

    g_log.log("make frame");
    g_log.log_package(ret.data(), ret.length());
    return ret;
}

std::string hk_led_make_cmd(const std::string &_msg, const std::string &_plate_no)
{
    std::string ret;
    unsigned char frame_header[] = {0x55, 0xaa, 0x00, 0x00, 0x01, 0x00, 0x00};
    unsigned char frame_tail[] = {0x00, 0x00, 0x0d, 0x0a};
    unsigned char op_code[] = {0xda};
    unsigned char frame_no[] = {0x00, 0x00};
    unsigned int data_len = 0;
    unsigned char frame_reserve[] = {0x00, 0x00};

    auto frame_data = hk_led_make_frame_data(_msg, _plate_no);
    if (frame_data.length() > 0)
    {
        ZH_HK_ORIGINAL_FRAME(ret, frame_header);
        ZH_HK_ORIGINAL_FRAME(ret, op_code);
        ZH_HK_ORIGINAL_FRAME(ret, frame_no);
        data_len = frame_data.length();
        ret.append((char *)&data_len, sizeof(data_len));
        ZH_HK_ORIGINAL_FRAME(ret, frame_reserve);
        ret.append((char *)&data_len, sizeof(data_len));
        ret.append(frame_data);
        ZH_HK_ORIGINAL_FRAME(ret, frame_tail);
    }
    g_log.log("make cmd");
    g_log.log_package(ret.data(), ret.length());
    return ret;
}

struct hk_led_param
{
    std::string led_ip;
    std::string cmd;
};

static void async_led_post(const std::string &_led_ip, const std::string &_cmd)
{
    auto led_param = new hk_led_param();
    led_param->led_ip = _led_ip;
    led_param->cmd = _cmd;
    tdf_main::get_inst().Async_to_workthread(
        [](void *_private, const std::string &chrct)
        {
            auto led_param = (hk_led_param *)(_private);
            hk_led_connector hkc(led_param->led_ip);
            hkc.send_cmd(led_param->cmd);
            delete led_param;
        },
        led_param, "");
}

bool zh_hk_cast_empty(const std::string &_led_ip)
{
    async_led_post(_led_ip, hk_led_make_cmd("", ""));
    return true;
}

void zh_hk_cast_auto_empty(int _second, const std::string &_led_ip)
{
    tdf_main::get_inst().start_timer(
        _second,
        [](void *_private)
        {
            auto led_ip = (std::string *)_private;
            zh_hk_cast_empty(*led_ip);
            free(_private);
        },
        new std::string(_led_ip), true);
}

bool zh_hk_cast_enter_scale(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(3, _led_ip);
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("请上磅", _plate_no));
}
bool zh_hk_cast_no_order(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(6, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd("未派车", _plate_no));
    return true;
}
bool zh_hk_cast_no_call(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(6, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd("未叫号", _plate_no));
    return true;
}
bool zh_hk_cast_raster_block(const std::string &_led_ip)
{
    async_led_post(_led_ip, hk_led_make_cmd("请注意，不要撞杆", ""));
    return true;
}
bool zh_hk_cast_no_confirm(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(6, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd("未确认装卸货", _plate_no));
    return true;
}
bool zh_hk_cast_holding(const std::string &_led_ip, const std::string &_plate_no)
{
    async_led_post(_led_ip, hk_led_make_cmd("请保持静止，等待提示", ""));
    return true;
}
bool zh_hk_cast_exit_scale(const std::string &_led_ip, const std::string &_weight, const std::string &_plate_no, bool need_ticket_cast)
{
    std::string cast_content = "请下磅,";
    if (need_ticket_cast)
    {
        cast_content = "请取称重小票后下磅,";
    }
    async_led_post(_led_ip, hk_led_make_cmd(_plate_no, _weight + "吨," + cast_content));
    return true;
}
bool zh_hk_cast_exit_busy(const std::string &_led_ip)
{
    zh_hk_cast_auto_empty(5, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd("正在称重，请稍后", ""));
    return true;
}

bool zh_hk_cast_welcome(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(8, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd("欢迎光临", _plate_no));
    return true;
}
bool zh_hk_cast_leave_timeout(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(8, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd("场内滞留时间过长，请联系管理员处理", _plate_no));
    return true;
}
bool zh_hk_cast_cannot_leave(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(8, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd("未确认装卸货或未二次称重，不能离场", _plate_no));
    return true;
}
bool zh_hk_cast_need_manual(const std::string &_led_ip, const std::string &_plate_no)
{
    async_led_post(_led_ip, hk_led_make_cmd("请联系管理员确认称重", _plate_no));
    return true;
}
bool zh_hk_cast_need_drop(const std::string &_led_ip, const std::string &_plate_no)
{
    async_led_post(_led_ip, hk_led_make_cmd("请下榜卸货后再上磅", _plate_no));
    return true;
}
bool zh_hk_cast_leave_bye(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(8, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd("一路顺风", _plate_no));
    return true;
}
bool zh_hk_cast_cannot_scale(const std::string &_led_ip, const std::string &_msg)
{
    zh_hk_cast_auto_empty(8, _led_ip);
    async_led_post(_led_ip, hk_led_make_cmd(_msg, ""));
    return true;
}

std::string zh_hk_get_channel_video(const std::string &_nvr_ip, int _channel_id, const NET_DVR_TIME &_start, const NET_DVR_TIME &_end, const std::string &_user_name, const std::string &_password)
{
    std::string ret;
    NET_DVR_DEVICEINFO_V30 tmp_info = {0};
    auto begin_point = time(NULL);
    auto user_id = NET_DVR_Login_V30(_nvr_ip.c_str(), 8000, _user_name.c_str(), _password.c_str(), &tmp_info);
    std::string store_prefix = "/manage_dist/logo_res/";
    std::string web_prefix = "/logo_res/";
    if (user_id >= 0)
    {
        NET_DVR_PLAYCOND time_cond = {0};
        time_cond.dwChannel = _channel_id + (tmp_info.byStartDChan - 1);
        time_cond.struStartTime = _start;
        time_cond.struStopTime = _end;
        std::string file_name = store_prefix + "vehicle_video_" + std::to_string(_channel_id) + "_" + std::to_string(time(NULL)) + ".mp4";
        auto find_video_ret = NET_DVR_GetFileByTime_V40(user_id, (char *)(file_name.c_str()), &time_cond);
        if (find_video_ret >= 0)
        {
            if (NET_DVR_PlayBackControl_V40(find_video_ret, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL))
            {
                int nPos = 0;
                for (nPos = 0; nPos < 100 && nPos >= 0; nPos = NET_DVR_GetDownloadPos(find_video_ret))
                {
                    g_log.log("Be downloading... %d %%\n", nPos);
                    usleep(5000); // millisecond
                }
                if (NET_DVR_StopGetFile(find_video_ret) && nPos == 100)
                {
                    ret = file_name;
                }
                else
                {
                    printf("failed to stop get file [%d]", NET_DVR_GetLastError());
                }
            }
            else
            {
                g_log.err("Play back control failed [%d]", NET_DVR_GetLastError());
            }
        }
        else
        {
            g_log.err("NET_DVR_GetFileByTime_V40 fail,last error %d", NET_DVR_GetLastError());
        }

        NET_DVR_Logout_V30(user_id);
    }
    else
    {
        g_log.err("failed to LOGIN device:%d", NET_DVR_GetLastError());
    }
    if (ret.length() > 0)
    {
        std::string cmd = "ffmpeg -threads 4 -i " + ret + " -s 320x180 tmp_video.mp4 && mv tmp_video.mp4 " + ret;
        system(cmd.c_str());
    }

    auto end_point = time(NULL);
    g_log.log("convert takes %d second", end_point - begin_point);
    if (ret.length() > store_prefix.length())
    {
        ret = ret.substr(store_prefix.length(), ret.length() - store_prefix.length());
    }

    return "https://" + std::string(getenv("BASE_URL")) + std::string(getenv("URL_REMOTE")) + web_prefix + ret;
}

std::string zh_hk_get_capture_picture(const std::string &_nvr_ip, int _channel_id, const std::string _user_name, const std::string &_password)
{
    std::string ret;
    auto begin_point = time(NULL);
    NET_DVR_DEVICEINFO_V30 tmp_info = {0};
    auto user_id = NET_DVR_Login_V30(_nvr_ip.c_str(), 8000, _user_name.c_str(), _password.c_str(), &tmp_info);
    if (user_id >= 0)
    {
        NET_DVR_JPEGPARA cap_param = {
            .wPicSize = 0xff,
            .wPicQuality = 1};
        std::string store_prefix = "/manage_dist";
        ret = "/logo_res/pic_" + std::to_string(time(NULL)) + "_" + _nvr_ip + "_" + std::to_string(_channel_id) + ".jpeg";
        if (TRUE == NET_DVR_CaptureJPEGPicture(user_id, _channel_id + (tmp_info.byStartDChan - 1), &cap_param, (char *)((store_prefix + ret).c_str())))
        {
            g_log.log("success cap picture:%s", ret.c_str());
            auto chmod_opt = "chmod +r " + store_prefix + ret;
            system(chmod_opt.c_str());
        }
        else
        {
            g_log.err("failed to cap picture [%d]", NET_DVR_GetLastError());
        }
        NET_DVR_Logout_V30(user_id);
    }
    else
    {
        g_log.err("failed to LOGIN device:%d", NET_DVR_GetLastError());
    }
    auto end_point = time(NULL);
    g_log.log("cap pic spend %d second", end_point - begin_point);

    return "https://" + std::string(getenv("BASE_URL")) + std::string(getenv("URL_REMOTE")) + ret;
}

void zh_hk_reboot_cam(const std::string &_ip)
{
    NET_DVR_DEVICEINFO_V30 tmp_info = {0};
    auto user_id = NET_DVR_Login_V30(_ip.c_str(), 8000, "admin", "P@ssw0rd", &tmp_info);
    if (user_id >= 0)
    {
        NET_DVR_RebootDVR(user_id);
    }
    else
    {
        g_log.err("failed to LOGIN device:%d", NET_DVR_GetLastError());
    }
}