#include "../abs_device_driver.h"
extern "C"
{
#include "VzLPRClientSDK.h"
}
static int g_zc_handler = -1;
static long long g_zc_ser_handler = -1;
static const uint8_t _CRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40};
static const uint8_t _CRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xd3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40};

uint16_t MB_CRC16(uint8_t *pFrame, uint16_t count)
{
    uint8_t CRCHi = 0xFF;
    uint8_t CRCLo = 0xFF;
    int32_t index;
    while (count--)
        index = CRCLo ^ *(pFrame++);
    CRCLo = (uint8_t)(CRCHi ^ _CRCHi[index]);
    CRCHi = _CRCLo[index];
    return (uint16_t)(CRCHi << 8 | CRCLo);
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
    data.push_back(0xff);
    data.push_back(0xff);
    data.push_back(0xff);
    data.push_back(0xff);
    data.push_back(0xff);
    data.push_back(0xff);
    data.push_back(0xff);

    auto gbd_content = utf2gbk(_content);
    unsigned short cont_len = gbd_content.length();
    data.push_back(*(char *)(&cont_len));
    data.append(gbd_content);

    return data;
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
    ret.push_back(*((char *)(&cont_len) + 1));
    ret.append(_data);

    auto crc_value = MB_CRC16((unsigned char *)(ret.data()), ret.length());
    ret.push_back(*(char *)(&crc_value));
    ret.push_back(*((char *)(&crc_value) + 1));

    tdf_log pack_log("led");
    pack_log.log_package(ret.data(), ret.length());

    std::string base_64_ret;
    Base64::Encode(ret, &base_64_ret);

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
    virtual void led_show(const std::string &_l1, const std::string &_l2, const std::string &_l3, const std::string &_l4) {
        auto l1_frame = make_frame(0x62, make_oneline_text(_l1, 0));
        auto l2_frame = make_frame(0x62, make_oneline_text(_l2, 1));
        auto l3_frame = make_frame(0x62, make_oneline_text(_l3, 2));
        auto l4_frame = make_frame(0x62, make_oneline_text(_l4, 3));
        auto zs_ret = VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(l1_frame.c_str()), l1_frame.length());
        zs_ret |= VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(l2_frame.c_str()), l1_frame.length());
        zs_ret |= VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(l3_frame.c_str()), l1_frame.length());
        zs_ret |= VzLPRClient_SerialSend(g_zc_ser_handler, (unsigned char *)(l4_frame.c_str()), l1_frame.length());
        if (0 != zs_ret)
        {
            PRINT_ERR("failed to send ser data:%d", zs_ret);
        }
    }

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
        if (1 == val)
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
        pzs->pub_device_status(true);
        pzs->pub_vehicle_come(utf_plate);
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
                auto ser_handler = VzLPRClient_SerialStart_V2(handler, 0, [](int, const unsigned char *_resp_data, unsigned _resp_len, void *) {
                    tdf_log pack_log("led");
                    pack_log.log_package((const char *)_resp_data, _resp_len);
                }, nullptr);
                if (ser_handler != 0)
                {
                    g_zc_ser_handler = ser_handler;
                }
                else
                {
                    PRINT_ERR("failed to open ser port:%d", ser_handler);
                }
                g_zc_handler = handler;
                if (0 == (zs_ret = VzLPRClient_SetPlateInfoCallBack(handler, plate_callback, &zc, false)))
                {
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
