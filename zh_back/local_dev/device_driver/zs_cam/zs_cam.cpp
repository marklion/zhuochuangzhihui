#include "../abs_device_driver.h"
extern "C"
{
#include "VzLPRClientSDK.h"
}
static int g_zc_handler = -1;
static std::string make_frame(unsigned char _cmd, const std::string &_data)
{
    std::string ret;

    return ret;
}
static std::string make_oneline_text(const std::string &_content, int _line)
{
    std::string ret;
    std::string data;
    unsigned char cmd = 0x62;
    data.push_back((char)_line);
    data.push_back(0);
    data.push_back(16);
    data.push_back(0);

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
        if (0 == VzLPRClient_SaveSnapImageToJpeg(g_zc_handler, pic_path.c_str()))
        {
            ret = pic_path;
        }

        return ret;
    }
    virtual void manual_trigger()
    {
        VzLPRClient_ForceTrigger(g_zc_handler);
    }

    // 屏幕能力
    virtual void led_show(const std::string &_l1, const std::string &_l2, const std::string &_l3, const std::string &_l4) {}

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
        if (_make_close)
        {
            VzLPRClient_SetIOOutputAuto(g_zc_handler, 1, 1000);
        }
        else
        {
            VzLPRClient_SetIOOutputAuto(g_zc_handler, 0, 1000);
        }
    }
    virtual bool gate_is_close()
    {
        bool ret = false;

        int val = 1;
        VzLPRClient_GetGPIOValue(g_zc_handler, 0, &val);
        if (0 == val)
        {
            ret = true;
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
        pzs->pub_device_status(true);
        pzs->pub_vehicle_come(plate);
    }

    return 0;
}
int main(int argc, char const *argv[])
{
    zs_cam zc(argc, argv, {"ip", "port", "username", "password"});
    bool healthy = false;
    if (0 == VzLPRClient_Setup())
    {
        if (0 == VZLPRClient_SetCommonNotifyCallBack(common_callback, &healthy))
        {
            auto handler = VzLPRClient_Open(zc.get_device_arg("ip").c_str(), (unsigned short)atoi(zc.get_device_arg("port").c_str()), zc.get_device_arg("username").c_str(), zc.get_device_arg("password").c_str());
            if (handler > 0)
            {
                g_zc_handler = handler;
                if (0 == VzLPRClient_SetPlateInfoCallBack(handler, plate_callback, &zc, false))
                {
                    while (healthy)
                    {
                        zc.proc_msg();
                    }
                }
            }
            VzLPRClient_Close(handler);
        }
        VzLPRClient_Cleanup();
    }
    sleep(15);
    zc.pub_device_status(false);

    return 0;
}
