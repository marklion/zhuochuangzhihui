#include "zh_scale.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include "../../zh_tdf/tdf_include.h"

static tdf_log g_log("scale");

double get_current_weight(const std::string &_scale_ip, unsigned short _port)
{
    double ret = 0.0001;
    zh_vcom_link vl(_scale_ip, _port);
    if (!vl.proc_modbus(
            1,
            [&](modbus_t *mctx, void *_private) -> bool
            {
                bool lamda_ret = false;
                unsigned short tmp[2];
                if (2 == modbus_read_registers(mctx, 4, 2, tmp))
                {
                    ret = modbus_get_float_abcd(tmp);
                    lamda_ret = true;
                }
                else
                {
                    g_log.err("failed to read scale register: %s", modbus_strerror(errno));
                }
                return lamda_ret;
            },
            nullptr))
    {
        g_log.err("failed to read scale");
    }

    return ret;
}
void clean_scale_weight(const std::string &_scale_ip, unsigned short _port)
{
    zh_vcom_link vl(_scale_ip, _port);
    if (!vl.proc_modbus(
            1,
            [&](modbus_t *mctx, void *_private) -> bool
            {
                if (1 == modbus_write_bit(mctx, 0, TRUE))
                {
                    return true;
                }
                else
                {
                    g_log.err("failed to zero scale:%s", modbus_strerror(errno));
                }
                return false;
            },
            nullptr))
    {
        g_log.err("failed to set scale");
    }
}