#include "zh_raster.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include "../../zh_tdf/tdf_include.h"
#include <modbus/modbus.h>

static tdf_log g_log("raster");

bool raster_was_block(const std::string &ip, unsigned short port)
{
    bool ret = true;

    zh_vcom_link vl(ip, port);
    auto mctx = modbus_new_rtu(vl.get_pts().c_str(), 19200, 'E', 8, 1);
    if (mctx)
    {
        if (0 == modbus_set_slave(mctx, 15))
        {
            unsigned int sec;
            unsigned int usec;
            if (0 == modbus_get_response_timeout(mctx, &sec, &usec))
            {
                g_log.log("get response timeout sec:%d, usec:%d", sec, usec);
                sec = 10;
                usec = 0;
                if (0 == modbus_set_response_timeout(mctx, sec, usec))
                {
                    g_log.log("set response timeout sec:%d, usec:%d", sec, usec);
                }
            }
            if (0 == modbus_connect(mctx))
            {
                unsigned short reply[24] = {0};
                auto rc = modbus_read_registers(mctx, 0, 2, reply);
                if (rc > 0)
                {
                    unsigned short fin_ret = 0;
                    for (auto &itr : reply)
                    {
                        fin_ret |= itr;
                    }
                    if (fin_ret == 0)
                    {
                        ret = false;
                    }
                }
                else
                {
                    g_log.err("fail to read raster:%s", modbus_strerror(errno));
                }
                modbus_close(mctx);
            }
            else
            {
                g_log.err("fail to connect raster:%s", modbus_strerror(errno));
            }
        }
        else
        {
            g_log.err("fail to set_slave raster:%s", modbus_strerror(errno));
        }

        modbus_free(mctx);
    }
    else
    {
        g_log.err("fail to open raster:%s", modbus_strerror(errno));
    }

    return ret;
}