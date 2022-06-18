#include "zh_raster.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include "../../zh_tdf/tdf_include.h"
#include "../../zh_database/zh_db_config.h"
#include <modbus/modbus.h>

static tdf_log g_log("raster");

std::string int_2_bin_string(int x)
{
    std::string ret = "";

    while (x)
    {
        if (x%2 == 0)
        {
            ret = "0" + ret;
        }
        else
        {
            ret = "1" + ret;
        }
        x /= 2;
    }

    return ret;
}

bool raster_was_block(const std::string &ip, unsigned short port)
{
    bool ret = false;
    bool read_ret = false;

    zh_vcom_link vl(ip, port);

    vl.proc_modbus(
        15,
        [&](modbus_t *_mctx, void *_private) -> bool
        {
            unsigned short reply[24] = {0};
            auto rc = modbus_read_registers(_mctx, 0, 2, reply);
            if (rc > 0)
            {
                unsigned char buff[4] = {0};
                memcpy(buff, reply, sizeof(buff));
                buff[0] ^= buff[1];
                buff[1] ^= buff[0];
                buff[0] ^= buff[1];
                buff[2] ^= buff[3];
                buff[3] ^= buff[2];
                buff[2] ^= buff[3];
                g_log.log("%s,%s,%s,%s", int_2_bin_string(buff[0]).c_str(), int_2_bin_string(buff[1]).c_str(), int_2_bin_string(buff[2]).c_str(), int_2_bin_string(buff[3]).c_str());
                g_log.log_package((char *)buff, sizeof(buff));
                g_log.log("%04X,%04X", reply[0],reply[1]);
                if ((buff[1] & 0x03)  == 0)
                {
                    ret = false;
                }
                else
                {
                    ret = true;
                }
                read_ret = true;
            }
            else
            {
                g_log.err("fail to read raster:%s", modbus_strerror(errno));
            }
            return true;
        },
        nullptr);

    if (read_ret)
    {
        zh_runtime_get_device_health()[ip] = 1;
    }
    else
    {
        zh_runtime_get_device_health()[ip] = 2;
    }

    return ret;
}