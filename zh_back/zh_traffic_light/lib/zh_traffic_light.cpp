#include "zh_traffic_light.h"
#include "../../zh_tdf/tdf_include.h"
#include "../../zh_database/zh_db_config.h"
#include <modbus/modbus.h>

static tdf_log g_log("traffic_light");

static bool set_modbus_light(const std::string &_ip, bool _close)
{
    bool ret = false;
    auto mtx = modbus_new_tcp(_ip.c_str(), MODBUS_TCP_DEFAULT_PORT);
    if (mtx)
    {
        modbus_set_byte_timeout(mtx, 3, 0);
        modbus_set_response_timeout(mtx, 3, 0);
        if (0 == modbus_connect(mtx))
        {
            if (1 == modbus_write_bit(mtx, 0, _close))
            {
                ret = true;
            }
            else
            {
                g_log.err("failed to set coil modbus:%s", modbus_strerror(errno));
            }
            modbus_close(mtx);
        }
        else
        {
            g_log.err("failed to connect modbus:%s", modbus_strerror(errno));
        }
        modbus_free(mtx);
    }
    else
    {
        g_log.err("failed to open tcp modbus:%s", modbus_strerror(errno));
    }
    return ret;
}

bool ZH_TRLI_set_red(const std::string &_ip)
{
    return set_modbus_light(_ip, false);
}
bool ZH_TRLI_set_green(const std::string &_ip)
{
    return set_modbus_light(_ip, true);
}