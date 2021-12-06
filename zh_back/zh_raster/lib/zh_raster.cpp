#include "zh_raster.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include "../../zh_tdf/tdf_include.h"
#include <modbus/modbus.h>

static tdf_log g_log("raster");

bool raster_was_block(const std::string &ip, unsigned short port)
{
    bool ret = false;

    zh_vcom_link vl(ip, port);

    vl.proc_modbus(
        15,
        [&](modbus_t *_mctx, void *_private) -> bool
        {
            unsigned short reply[24] = {0};
            auto rc = modbus_read_registers(_mctx, 0, 2, reply);
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
                else
                {
                    ret = true;
                }
            }
            else
            {
                g_log.err("fail to read raster:%s", modbus_strerror(errno));
            }
            return true;
        },
        nullptr);

    return ret;
}