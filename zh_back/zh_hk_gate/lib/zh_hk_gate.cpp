#include "zh_hk_gate.h"
#include "../../zh_database/CJsonObject.hpp"
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

bool zh_hk_subcribe_event(const std::string &_road_ip, zh_sub_callback_cfg _callback_cfg)
{
    bool ret = false;
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
                    if (plate_no.length() > 2)
                    {
                        plate_no = plate_no.substr(2, plate_no.length() - 2);
                    }
                    plate_no = gbk2utf(plate_no);
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

bool zh_hk_ctrl_gate(const std::string &_road_ip, zh_hk_gate_control_cmd _cmd)
{
    bool ret = false;

    auto hk_duh = g_device_user_map[_road_ip];
    if (hk_duh.user_id != -1)
    {
        NET_DVR_BARRIERGATE_CFG tmp = {0};
        tmp.dwSize = sizeof(tmp);
        tmp.dwChannel = 1;
        tmp.byLaneNo = 1;
        tmp.byBarrierGateCtrl = _cmd;
        ret = NET_DVR_RemoteControl(hk_duh.user_id, NET_DVR_BARRIERGATE_CTRL, &tmp, sizeof(tmp));
        if (ret)
        {
            g_log.log("open door %s", _road_ip.c_str());
            zh_runtime_get_device_health()[_road_ip] = 1;
        }
        else
        {
            g_log.err("open door %s, error_code:%d", _road_ip.c_str(), NET_DVR_GetLastError());
            zh_runtime_get_device_health()[_road_ip] = 2;
        }
    }

    return ret;
}

struct hk_led_connector
{
    int socket_fd = -1;
    std::string m_ip;
    hk_led_connector(const std::string &_ip):m_ip(_ip)
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

bool zh_hk_ctrl_led(const std::string &_led_ip, zh_hk_led_type content_type, const std::string &_vehicle_number)
{
    bool ret = false;
    std::string gb_vn = utf2gbk(_vehicle_number);
    std::string out_buff;
    switch (content_type)
    {
    case zh_hk_led_enter_gate:
    {
        unsigned char buff[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0xDB, 0x00, 0x00, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x00, 0x00, 0x01, 0x01, 0xB3, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x0F, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xD7, 0xBF, 0xB4, 0xB4, 0xD6, 0xC7, 0xBB, 0xE3, 0x02, 0x35, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x10, 0x00, 0x3F, 0x00, 0x1F, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x0A, 0x0A, 0x10, 0x1B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x59, 0x34, 0x2D, 0x25, 0x4D, 0x33, 0x2D, 0x25, 0x44, 0x32, 0x20, 0x25, 0x68, 0x30, 0x3A, 0x25, 0x6D, 0x3A, 0x25, 0x73, 0x03, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x20, 0x00, 0x3F, 0x00, 0x2F, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xC3, 0xC9, 0x51, 0x5A, 0x4E, 0x33, 0x37, 0x30, 0x04, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x30, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xBB, 0xB6, 0xD3, 0xAD, 0xB9, 0xE2, 0xC1, 0xD9, 0x00, 0x00, 0x0D, 0x0A};
        out_buff.append((char *)buff, sizeof(buff));
        break;
    }
    case zh_hk_led_exit_gate:
    {
        unsigned char buff[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0xDB, 0x00, 0x00, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x00, 0x00, 0x01, 0x01, 0xB3, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x0F, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xD7, 0xBF, 0xB4, 0xB4, 0xD6, 0xC7, 0xBB, 0xE3, 0x02, 0x35, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x10, 0x00, 0x3F, 0x00, 0x1F, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x0A, 0x0A, 0x10, 0x1B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x59, 0x34, 0x2D, 0x25, 0x4D, 0x33, 0x2D, 0x25, 0x44, 0x32, 0x20, 0x25, 0x68, 0x30, 0x3A, 0x25, 0x6D, 0x3A, 0x25, 0x73, 0x03, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x20, 0x00, 0x3F, 0x00, 0x2F, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xC3, 0xC9, 0x51, 0x5A, 0x4E, 0x33, 0x37, 0x30, 0x04, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x30, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xD2, 0xBB, 0xC2, 0xB7, 0xCB, 0xB3, 0xB7, 0xE7, 0x00, 0x00, 0x0D, 0x0A};
        out_buff.append((char *)buff, sizeof(buff));
        break;
    }
    case zh_hk_led_enter_scale:
    {
        unsigned char buff[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0xDB, 0x00, 0x00, 0xB2, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB2, 0x00, 0x00, 0x00, 0x01, 0x01, 0xB1, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x0F, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xD7, 0xBF, 0xB4, 0xB4, 0xD6, 0xC7, 0xBB, 0xE3, 0x02, 0x35, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x10, 0x00, 0x3F, 0x00, 0x1F, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x0A, 0x0A, 0x10, 0x1B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x59, 0x34, 0x2D, 0x25, 0x4D, 0x33, 0x2D, 0x25, 0x44, 0x32, 0x20, 0x25, 0x68, 0x30, 0x3A, 0x25, 0x6D, 0x3A, 0x25, 0x73, 0x03, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x20, 0x00, 0x3F, 0x00, 0x2F, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xC3, 0xC9, 0x51, 0x5A, 0x4E, 0x33, 0x37, 0x30, 0x04, 0x20, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x30, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x06, 0x00, 0x00, 0x00, 0xC7, 0xEB, 0xC9, 0xCF, 0xB0, 0xF5, 0x00, 0x00, 0x0D, 0x0A};
        out_buff.append((char *)buff, sizeof(buff));
        break;
    }
    case zh_hk_led_exit_scale:
    {
        unsigned char buff[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0xDB, 0x00, 0x00, 0xB2, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB2, 0x00, 0x00, 0x00, 0x01, 0x01, 0xB1, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x0F, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xD7, 0xBF, 0xB4, 0xB4, 0xD6, 0xC7, 0xBB, 0xE3, 0x02, 0x35, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x10, 0x00, 0x3F, 0x00, 0x1F, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x0A, 0x0A, 0x10, 0x1B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x59, 0x34, 0x2D, 0x25, 0x4D, 0x33, 0x2D, 0x25, 0x44, 0x32, 0x20, 0x25, 0x68, 0x30, 0x3A, 0x25, 0x6D, 0x3A, 0x25, 0x73, 0x03, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x20, 0x00, 0x3F, 0x00, 0x2F, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xC3, 0xC9, 0x51, 0x5A, 0x4E, 0x33, 0x37, 0x30, 0x04, 0x20, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x30, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x06, 0x00, 0x00, 0x00, 0xC7, 0xEB, 0xCF, 0xC2, 0xB0, 0xF5, 0x00, 0x00, 0x0D, 0x0A};
        out_buff.append((char *)buff, sizeof(buff));
        break;
    }
    case zh_hk_led_cannot_enter_scale:
    {
        unsigned char buff[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0xDB, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x01, 0x01, 0xC7, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x0F, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xD7, 0xBF, 0xB4, 0xB4, 0xD6, 0xC7, 0xBB, 0xE3, 0x02, 0x35, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x10, 0x00, 0x3F, 0x00, 0x1F, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x0A, 0x0A, 0x10, 0x1B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x59, 0x34, 0x2D, 0x25, 0x4D, 0x33, 0x2D, 0x25, 0x44, 0x32, 0x20, 0x25, 0x68, 0x30, 0x3A, 0x25, 0x6D, 0x3A, 0x25, 0x73, 0x03, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x20, 0x00, 0x3F, 0x00, 0x2F, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xC3, 0xC9, 0x51, 0x5A, 0x4E, 0x33, 0x37, 0x30, 0x04, 0x36, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x30, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x1C, 0x00, 0x00, 0x00, 0xCE, 0xB4, 0xD7, 0xB0, 0xD0, 0xB6, 0xBB, 0xF5, 0xC8, 0xB7, 0xC8, 0xCF, 0xA3, 0xAC, 0xB2, 0xBB, 0xD4, 0xCA, 0xD0, 0xED, 0xB6, 0xFE, 0xB4, 0xCE, 0xB3, 0xC6, 0xD6, 0xD8, 0x00, 0x00, 0x0D, 0x0A};
        out_buff.append((char *)buff, sizeof(buff));
        break;
    }
    case zh_hk_led_cannot_exit_gate:
    {
        unsigned char buff[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0xDB, 0x00, 0x00, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB6, 0x00, 0x00, 0x00, 0x01, 0x01, 0xB5, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x0F, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xD7, 0xBF, 0xB4, 0xB4, 0xD6, 0xC7, 0xBB, 0xE3, 0x02, 0x35, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x10, 0x00, 0x3F, 0x00, 0x1F, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x0A, 0x0A, 0x10, 0x1B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x59, 0x34, 0x2D, 0x25, 0x4D, 0x33, 0x2D, 0x25, 0x44, 0x32, 0x20, 0x25, 0x68, 0x30, 0x3A, 0x25, 0x6D, 0x3A, 0x25, 0x73, 0x03, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x20, 0x00, 0x3F, 0x00, 0x2F, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xC3, 0xC9, 0x51, 0x5A, 0x4E, 0x33, 0x37, 0x30, 0x04, 0x24, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x30, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x0A, 0x00, 0x00, 0x00, 0xCE, 0xB4, 0xCD, 0xEA, 0xB3, 0xC9, 0xB3, 0xC6, 0xD6, 0xD8, 0x00, 0x00, 0x0D, 0x0A};
        out_buff.append((char *)buff, sizeof(buff));
        break;
    }

    default:
        break;
    }
    if (out_buff.size() > 158 && gb_vn.size() == 8)
    {
        char *buff = new char[out_buff.size()];
        memcpy(buff, out_buff.data(), out_buff.size());
        memcpy(buff + 158, gb_vn.data(), gb_vn.size());
        hk_led_connector hkc(_led_ip);
        ret = hkc.send_cmd(std::string(buff, out_buff.size()));
        delete[] buff;
    }

    return ret;
}
bool zh_hk_ctrl_led(const std::string &_led_ip, const std::string &_content_utf)
{
    auto _content = utf2gbk(_content_utf);
    bool ret = false;
    hk_led_connector hkc(_led_ip);
    unsigned char sample_buff[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0xDB, 0x00, 0x00, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00, 0x93, 0x00, 0x00, 0x00, 0x01, 0x01, 0x92, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x0F, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x08, 0x00, 0x00, 0x00, 0xD7, 0xBF, 0xB4, 0xB4, 0xD6, 0xC7, 0xBB, 0xE3, 0x02, 0x35, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x10, 0x00, 0x3F, 0x00, 0x1F, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x0A, 0x0A, 0x10, 0x1B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x59, 0x34, 0x2D, 0x25, 0x4D, 0x33, 0x2D, 0x25, 0x44, 0x32, 0x20, 0x25, 0x68, 0x30, 0x3A, 0x25, 0x6D, 0x3A, 0x25, 0x73, 0x03, 0x23, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x20, 0x00, 0x3F, 0x00, 0x2F, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x10, 0x09, 0x00, 0x00, 0x00};
    unsigned char tail[] = {0x00, 0x00, 0x0D, 0x0A};
    auto len_plus = _content.size() - 9;
    *(unsigned int *)(sample_buff + 10) += len_plus;
    *(unsigned int *)(sample_buff + 16) += len_plus;
    *(unsigned int *)(sample_buff + 22) += len_plus;
    *(unsigned int *)(sample_buff + 133) += len_plus;
    *(unsigned int *)(sample_buff + 154) += len_plus;
    std::string out_buff;
    out_buff.append((char *)sample_buff, sizeof(sample_buff));
    out_buff.append(_content.data(), _content.size());
    out_buff.append((char *)tail, sizeof(tail));
    ret = hkc.send_cmd(out_buff);

    return ret;
}
bool zh_hk_ctrl_voice(const std::string &_led_ip, const std::string &_content_utf)
{
    auto _content = utf2gbk(_content_utf);
    bool ret = false;
    hk_led_connector hkc(_led_ip);
    unsigned char sample_buff[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0xDE, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x01, 0x01, 0x1B, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};

    unsigned char tail[] = {0x00, 0x00, 0x0D, 0x0A};
    auto len_plus = _content.size();
    *(unsigned int *)(sample_buff + 10) += len_plus;
    *(unsigned int *)(sample_buff + 16) += len_plus;
    *(unsigned int *)(sample_buff + 22) += len_plus;
    *(unsigned int *)(sample_buff + 24) += len_plus;

    std::string out_buff;
    out_buff.append((char *)sample_buff, sizeof(sample_buff));
    out_buff.append(_content.data(), _content.size());
    out_buff.append((char *)tail, sizeof(tail));
    ret = hkc.send_cmd(out_buff);
    return ret;
}

void __attribute__((constructor)) zh_hk_init(void)
{
    // neb::CJsonObject req;
    // req.AddEmptySubArray("eventTypes");
    // req["eventTypes"].Add(771760130);
    // req["eventTypes"].Add(771760133);
    // req.Add("eventDest", "http://192.168.2.105/zh_rest/vehicle_event");
    // call_hk_post("/api/eventService/v1/eventSubscriptionByEventTypes", req);
    NET_DVR_Init();
    if (!NET_DVR_SetConnectTime(2000, 1) ||
        !NET_DVR_SetReconnect(10000, true) ||
        !NET_DVR_SetRecvTimeOut(30000))
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

#define ZH_HK_ORIGINAL_FRAME(ret,frame) (ret).append((char *)(frame), sizeof(frame))
std::string hk_led_make_text_block(const std::string &_msg, char _pos, char _color, char _action = 0x01)
{
    std::string ret;
    unsigned char zone_no[] = {_pos + 1};
    int zone_len = 26;
    unsigned char zone_type[] = {0x0e};
    unsigned char zone_position[] = {0x00,0x00,16*_pos,0x00,0x3f,0x00,0x0f + 16*_pos,0x00};
    unsigned char zone_color[] = {_color};
    unsigned char reserved[2] = {0};
    unsigned char zone_action[] = {_action, 0x00};
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
    std::string text_string(getenv("OEM_SHORT")?getenv("OEM_SHORT"):getenv("OEM_NAME"));
    return hk_led_make_text_block(text_string, 0, 1);
}
std::string hk_led_make_program_time_data()
{
    std::string ret;
    auto time_string = zh_rpc_util_get_timestring();
    ret = hk_led_make_text_block(time_string, 1, 8, 0x20);
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
    return hk_led_make_text_block(_plate_no, 3, 8);
}

std::string hk_led_make_program_voice_data(const std::string &_voice)
{
    std::string ret;
    unsigned char zone_no[] = {0x05};
    int zone_len = 27;
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
    unsigned char zone_count[] = {0x02};
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
    program_blocks += hk_led_make_program_time_data();
    if (_msg.length() > 0)
    {
        program_blocks += hk_led_make_program_msg_data(_msg);
        if (_plate_no.length() > 0)
        {
            program_blocks += hk_led_make_program_plate_data(_plate_no);
        }
        program_blocks += hk_led_make_program_voice_data(_msg + _plate_no);
    }
    program_len = program_blocks.length();

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

    if (_msg.length() > 0)
    {
        op_code[0] = 0xdb;
    }
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

bool zh_hk_cast_empty(const std::string &_led_ip)
{
    hk_led_connector hkc(_led_ip);
    unsigned char sample_date[]{0x55, 0xaa, 0x00, 0x00, 0x01, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x21, 0x01, 0x12, 0x20, 0x10, 0x32, 0x33, 0x00, 0x00, 0x0d, 0x0a};
    time_t cur_time;
    time(&cur_time);
    tm time_str;
    localtime_r(&cur_time, &time_str);
    sample_date[20] = ((time_str.tm_year + 1900 - 2000) % 10) | ((time_str.tm_year + 1900 - 2000) / 10 * 16);
    sample_date[21] = time_str.tm_wday;
    sample_date[22] = ((time_str.tm_mon + 1) % 10) | ((time_str.tm_mon + 1) / 10 * 16);
    sample_date[23] = (time_str.tm_mday % 10) | (time_str.tm_mday / 10 * 16);
    sample_date[24] = (time_str.tm_hour % 10) | (time_str.tm_hour / 10 * 16);
    sample_date[25] = (time_str.tm_min % 10) | (time_str.tm_min / 10 * 16);
    sample_date[26] = (time_str.tm_sec % 10) | (time_str.tm_sec / 10 * 16);
    std::string time_cor_cmd((char *)sample_date, sizeof(sample_date));
    hkc.send_cmd(time_cor_cmd);
    return hkc.send_cmd(hk_led_make_cmd("", ""));
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
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("未派车", _plate_no));
}
bool zh_hk_cast_no_call(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(6, _led_ip);
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("未叫号", _plate_no));
}
bool zh_hk_cast_no_confirm(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(6, _led_ip);
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("未确认装卸货", _plate_no));
}
bool zh_hk_cast_holding(const std::string &_led_ip)
{
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("请保持静止，等待提示", ""));
}
bool zh_hk_cast_exit_scale(const std::string &_led_ip, const std::string &_weight)
{
    zh_hk_cast_auto_empty(10, _led_ip);
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("请取称重小票后下榜", _weight + "吨"));
}
bool zh_hk_cast_exit_busy(const std::string &_led_ip)
{
    zh_hk_cast_auto_empty(3, _led_ip);
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("正在称重，请稍后", ""));
}

bool zh_hk_cast_welcome(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(3, _led_ip);
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("欢迎光临", _plate_no));
}
bool zh_hk_cast_cannot_leave(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(6, _led_ip);
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("未确认装卸货或未二次称重，不能离场", _plate_no));
}
bool zh_hk_cast_leave_bye(const std::string &_led_ip, const std::string &_plate_no)
{
    zh_hk_cast_auto_empty(3, _led_ip);
    hk_led_connector hkc(_led_ip);

    return hkc.send_cmd(hk_led_make_cmd("一路顺风", _plate_no));
}

std::string zh_hk_get_channel_video(const std::string &_nvr_ip, int _channel_id, const NET_DVR_TIME &_start, const NET_DVR_TIME &_end)
{
    std::string ret;
    NET_DVR_DEVICEINFO_V30 tmp_info = {0};
    auto user_id = NET_DVR_Login_V30(_nvr_ip.c_str(), 8000, "admin", "P@ssw0rd", &tmp_info);
    std::string store_prefix = "/manage_dist/logo_res/";
    if (user_id >= 0)
    {
        NET_DVR_PLAYCOND time_cond = {0};
        time_cond.dwChannel = _channel_id;
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
                    usleep(5000); //millisecond
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
        auto begin_point = time(NULL);
        std::string cmd = "ffmpeg -threads 4 -i " + ret + " -s 320x180 tmp_video.mp4 && mv tmp_video.mp4 " + ret;
        system(cmd.c_str());
        auto end_point = time(NULL);
        g_log.log("convert takes %d second", end_point - begin_point);
    }

    if (ret.length() > store_prefix.length())
    {
        ret = ret.substr(store_prefix.length(), ret.length() - store_prefix.length());
    }

    return ret;
}