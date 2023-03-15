#include "../abs_device_driver.h"
extern "C"
{
#include "VzLPRClientSDK.h"
}
static int g_zc_handler = -1;
static long long g_zc_ser_handler = -1;
static bool g_cur_green_light = true;
const uint16_t ModBusCRCTable[] = {
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040};

uint16_t ModBusCRC16(uint8_t *pData, int length)
{
    uint8_t index;
    uint16_t CRC = 0xFFFF;

    while (length--)
    {
        index = *pData++ ^ CRC;
        CRC >>= 8;
        CRC ^= ModBusCRCTable[index];
    }
    return CRC;
}
static std::string make_oneline_text(const std::string &_content, int _line)
{
    std::string data;
    data.push_back((char)_line);
    data.push_back(0x15);
    data.push_back(1);
    data.push_back(0);
    data.push_back(5);
    data.push_back(0x15);
    data.push_back(1);
    data.push_back(3);
    data.push_back(0);
    data.push_back(0xff);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);

    auto gbd_content = utf2gbk(_content);
    unsigned short cont_len = gbd_content.length();
    data.push_back(*(char *)(&cont_len));
    data.push_back(*((char *)(&cont_len) + 1));
    data.append(gbd_content);

    return data;
}

static std::string make_voice_content(const std::string &_voice)
{
    std::string ret;

    ret.push_back(1);
    ret.append(utf2gbk(_voice));

    return ret;
}

static std::string make_trfc_lt()
{
    std::string ret;

    ret.push_back(0);
    ret.push_back(0);
    ret.push_back(0);
    ret.push_back(0);
    ret.push_back(0);
    ret.push_back(1);

    return ret;
}

static std::string make_frame(unsigned char _cmd, const std::string &_data)
{
    std::string ret;

    ret.push_back(0);
    ret.push_back(100);
    ret.push_back(0xff);
    ret.push_back(0xff);
    ret.push_back(_cmd);

    unsigned short cont_len = _data.length();
    ret.push_back(*(char *)(&cont_len));
    ret.append(_data);

    auto crc_value = ModBusCRC16((unsigned char *)(ret.data()), ret.length());
    ret.push_back(*(char *)(&crc_value));
    ret.push_back(*((char *)(&crc_value) + 1));

    tdf_log pack_log("led");
    pack_log.log_package(ret.data(), ret.length());

    return ret;
}
class zs_cam : public abs_device_driver
{
public:
    using abs_device_driver::abs_device_driver;
    virtual std::string take_picture()
    {
        std::string ret;
        std::string pic_path = "/logo_res/plate_snap/" + self_name + "_" + local_dev_get_timestring() + ".jpg";
        int zs_ret = -1;
        if (0 == (zs_ret = VzLPRClient_SaveSnapImageToJpeg(g_zc_handler, ("/manage_dist" + pic_path).c_str())))
        {
            ret = pic_path;
        }
        else
        {
            PRINT_ERR("faile to snap:%d", zs_ret);
        }

        return ret;
    }
    virtual void manual_trigger()
    {
        auto zs_ret = VzLPRClient_ForceTrigger(g_zc_handler);
        if (zs_ret != 0)
        {
            PRINT_ERR("failed to force trigger:%d", zs_ret);
        }
    }

    // 屏幕能力
    virtual void led_show(const std::string &_l1, const std::string &_l2, const std::string &_l3, const std::string &_l4)
    {
        auto l1_frame = make_frame(0x62, make_oneline_text(_l1, 0));
        auto l2_frame = make_frame(0x62, make_oneline_text(_l2, 1));
        auto l3_frame = make_frame(0x62, make_oneline_text(_l3, 2));
        auto l4_frame = make_frame(0x62, make_oneline_text(_l4, 3));
        usleep(120000);
        auto zs_ret = VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(l1_frame.data()), l1_frame.length());
        usleep(120000);
        zs_ret |= VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(l2_frame.data()), l2_frame.length());
        usleep(120000);
        zs_ret |= VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(l3_frame.data()), l3_frame.length());
        usleep(120000);
        zs_ret |= VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(l4_frame.data()), l4_frame.length());
        if (0 != zs_ret)
        {
            PRINT_ERR("failed to send ser data:%d", zs_ret);
        }
    }

    // 喇叭能力
    virtual void speaker_speak(const std::string &_voice_content) {
        auto voice_req = make_frame(0x30, make_voice_content(_voice_content));
        usleep(120000);
        auto zs_ret = VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(voice_req.data()), voice_req.length());
        if (0 != zs_ret)
        {
            PRINT_ERR("failed to send ser data:%d", zs_ret);
        }
    }

    // 红绿灯能力
    virtual void traffic_light_set(bool _is_green) {
        g_cur_green_light = _is_green;
    }

    // 监控能力
    virtual std::string get_video_record(const std::string &_begin_date, const std::string &_end_date)
    {
        return "";
    }

    // 道闸能力
    virtual void gate_control(bool _make_close)
    {
        int zs_ret = -1;
        if (_make_close)
        {
            zs_ret = VzLPRClient_SetIOOutputAuto(g_zc_handler, 1, 1000);
        }
        else
        {
            zs_ret = VzLPRClient_SetIOOutputAuto(g_zc_handler, 0, 1000);
        }

        if (zs_ret != 0)
        {
            PRINT_ERR("failed to gate control:%d", zs_ret);
        }
    }
    virtual bool gate_is_close()
    {
        bool ret = false;

        int val = 1;
        auto zs_ret = VzLPRClient_GetGPIOValue(g_zc_handler, 0, &val);
        if (0 == val)
        {
            ret = true;
        }
        if (zs_ret != 0)
        {
            PRINT_ERR("failed to get gate status:%d", zs_ret);
        }

        return ret;
    }
};
void common_callback(VzLPRClientHandle handle, void *pUserData, VZ_LPRC_COMMON_NOTIFY eNotify, const char *pStrDetail)
{
    auto p_healthy = (bool *)pUserData;
    switch (eNotify)
    {
    case VZ_LPRC_ACCESS_DENIED:
    case VZ_LPRC_NETWORK_ERR:
    case VZ_LPRC_OFFLINE:
        *p_healthy = false;
        break;
    case VZ_LPRC_ONLINE:
        *p_healthy = true;
        break;
    default:
        break;
    }
    PRINT_ERR("%s", pStrDetail);
}
int plate_callback(VzLPRClientHandle handle, void *pUserData,
                   const TH_PlateResult *pResult, unsigned uNumPlates,
                   VZ_LPRC_RESULT_TYPE eResultType,
                   const VZ_LPRC_IMAGE_INFO *pImgFull,
                   const VZ_LPRC_IMAGE_INFO *pImgPlateClip)
{
    auto pzs = (zs_cam *)pUserData;
    std::string plate = pResult->license;
    if (plate.length() > 0)
    {
        auto utf_plate = gbk2utf(plate);
        if (utf_plate.find("无") == std::string::npos)
        {
            pzs->pub_device_status(true);
            pzs->pub_vehicle_come(utf_plate);
        }
    }

    return 0;
}
int main(int argc, char const *argv[])
{
    zs_cam zc(argc, argv, {"ip", "port", "username", "password", "boudrate"});
    bool healthy = true;
    int zs_ret = -1;

    if (0 == (zs_ret = VzLPRClient_Setup()))
    {
        if (0 == (zs_ret = VZLPRClient_SetCommonNotifyCallBack(common_callback, &healthy)))
        {
            auto handler = VzLPRClient_Open(zc.get_device_arg("ip").c_str(), (unsigned short)atoi(zc.get_device_arg("port").c_str()), zc.get_device_arg("username").c_str(), zc.get_device_arg("password").c_str());
            if (handler > 0)
            {
                VZ_SERIAL_PARAMETER ser_param;
                ser_param.uBaudRate = atoi(zc.get_device_arg("boudrate").c_str());
                ser_param.uDataBits = 8;
                ser_param.uParity = 0;
                ser_param.uStopBit = 1;
                if (0 != (zs_ret = VzLPRClient_SetSerialParameter(handler, 0, &ser_param)))
                {
                    PRINT_ERR("failed to set ser param:%d", zs_ret);
                }
                auto ser_handler = VzLPRClient_SerialStart_V2(
                    handler, 0, [](int, const unsigned char *_resp_data, unsigned _resp_len, void *)
                    {
                    tdf_log pack_log("led");
                    pack_log.log_package((const char *)_resp_data, _resp_len); },
                    nullptr);
                if (ser_handler != 0)
                {
                    g_zc_ser_handler = ser_handler;
                }
                else
                {
                    PRINT_ERR("failed to open ser port:%lld", ser_handler);
                }
                g_zc_handler = handler;
                if (0 == (zs_ret = VzLPRClient_SetPlateInfoCallBack(handler, plate_callback, &zc, false)))
                {
                    std::thread(
                        [&]()
                        {
                            while (1)
                            {
                                sleep(1);
                                if (g_cur_green_light)
                                {
                                    auto trfc_req = make_frame(0x0f, make_trfc_lt());
                                    VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(trfc_req.data()), trfc_req.length());
                                }
                            }
                        })
                        .detach();
                    while (healthy)
                    {
                        zc.proc_msg();
                    }
                }
                else
                {
                    PRINT_ERR("failed to set callback:%d", zs_ret);
                }
            }
            else
            {
                PRINT_ERR("failed to open device:%s %s %s %s", zc.get_device_arg("ip").c_str(), zc.get_device_arg("port").c_str(), zc.get_device_arg("username").c_str(), zc.get_device_arg("password").c_str());
            }
            VzLPRClient_Close(handler);
        }
        else
        {
            PRINT_ERR("failed to set common callback:%d", zs_ret);
        }
        VzLPRClient_Cleanup();
    }
    else
    {
        PRINT_ERR("failed to setup device:%d", zs_ret);
    }
    sleep(15);
    zc.pub_device_status(false);

    return 0;
}
