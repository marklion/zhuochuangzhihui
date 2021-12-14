#include "zh_qr_lib.h"

struct zh_qr_handler {
    std::string chrct;
    zh_sub_callback_cfg cfg;
};
static std::map<std::string, zh_qr_handler> g_callback_map;

bool zh_qr_subscribe(const std::string &_ip, zh_sub_callback_cfg _cfg)
{
    bool ret = false;
    if (g_callback_map.find(_ip) == g_callback_map.end())
    {
        std::string chrct;
        if (tdf_main::get_inst().connect_remote(
                _ip, ZH_QR_PORT,
                [&](const std::string &_chrct){
                    chrct = _chrct;
                }, nullptr,
                [](const std::string &_chrct, const std::string &_data)
                {
                    auto ip = _chrct.substr(0, _chrct.find_first_of(' '));
                    if (g_callback_map[ip].cfg.callback)
                    {
                        g_callback_map[ip].cfg.callback(_data, ip, g_callback_map[ip].cfg.pData);
                    }
                }))
        {
            zh_qr_handler tmp;
            tmp.cfg = _cfg;
            tmp.chrct = chrct;
            g_callback_map[_ip] = tmp;
            ret = true;
        }
    }

    return ret;
}
void zh_qr_unsubscribe(const std::string &_ip)
{
    if (g_callback_map.find(_ip) != g_callback_map.end())
    {
        tdf_main::get_inst().close_data(g_callback_map[_ip].chrct);
        g_callback_map.erase(_ip);
    }
}