#include "zh_scale.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include "../../zh_tdf/tdf_include.h"
#include <math.h>

static tdf_log g_log("scale");
static std::map<std::string, zh_scale_if *> g_scale_map;

std::unique_ptr<zh_scale_if> zh_scale_if::get_scale(const std::string &_brand)
{
    auto scale = g_scale_map[_brand];
    if (scale)
    {
        return scale->clone_self();
    }

    return std::unique_ptr<zh_scale_if>();
}

std::vector<std::string> zh_scale_if::get_all_brand()
{
    std::vector<std::string> ret;

    for (auto itr = g_scale_map.begin(); itr != g_scale_map.end();++itr)
    {
        ret.push_back(itr->first);
    }

    return ret;
}
class zh_scale_test:public zh_scale_if {
public:
    zh_scale_test() {
        std::string brand_name = "信衡";
        if (g_scale_map[brand_name] == NULL)
        {
            g_scale_map[brand_name] = this;
        }
    }
    virtual double get_weight(const std::string &_scale_ip, unsigned short _port)
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
    virtual void clean_scale(const std::string &_scale_ip, unsigned short _port)
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
    virtual std::unique_ptr<zh_scale_if> clone_self()
    {
        return std::unique_ptr<zh_scale_if>(new zh_scale_test(*this));
    }
} g_single_test_scale;

class zh_scale_kld2008 : public zh_scale_if
{
public:
    virtual double get_weight(const std::string &_scale_ip, unsigned short _port)
    {
        double ret = 0;
        zh_vcom_link vl(_scale_ip, _port);
        if (false == vl.proc_modbus(
                         1,
                         [&](modbus_t *mctx, void *_private) -> bool
                         {
                             bool lamda_ret = false;
                             char ret_buff[8] = {0};
                             if (sizeof(ret_buff) / 2 == modbus_read_registers(mctx, 1, sizeof(ret_buff) / 2, (unsigned short *)ret_buff))
                             {
                                 lamda_ret = true;
                                 int pos = 0;
                                 for (int i = 0; i <sizeof(ret_buff) / 2; i++)
                                 {
                                     auto tmp = ret_buff[i*2];
                                     ret_buff[i*2] = ret_buff[i*2+1];
                                     ret_buff[i*2+1] = tmp;
                                 }
                                 for (int i = sizeof(ret_buff) - 2; i >= 0; i--)
                                 {
                                     if (i == 0)
                                     {
                                         if (ret_buff[i] == '-')
                                         {
                                             ret = 0 - ret;
                                         }
                                         else
                                         {
                                             ret += (ret_buff[i] - '0') * pow(10, pos);
                                         }
                                     }
                                     else
                                     {
                                         ret += (ret_buff[i] - '0') * pow(10, pos);
                                     }
                                     pos++;
                                 }
                                 ret /= pow(10, (ret_buff[sizeof(ret_buff) - 1] - '0'));
                             }
                             else
                             {
                                 g_log.err("failed to read register:%s", modbus_strerror(errno));
                             }

                             return lamda_ret;
                         },
                         nullptr))
        {
            g_log.err("failed to read weight");
        }
        return ret;
    }
    virtual void clean_scale(const std::string &_scale_ip, unsigned short _port)
    {
        zh_vcom_link vl(_scale_ip, _port);
        if (false == vl.proc_modbus(
                         1, [&](modbus_t *mctx, void *_private) -> bool
                         {
                             bool ret = false;
                             if (1 == modbus_write_register(mctx, 1, 23))
                             {
                                 ret = true;
                             }
                             else
                             {
                                 g_log.err("failed to write register:%s", modbus_strerror(errno));
                             }
                             return ret;
                         },
                         nullptr))
        {
            g_log.err("failed to clean scale");
        }
    }
    virtual std::unique_ptr<zh_scale_if> clone_self()
    {
        return std::unique_ptr<zh_scale_kld2008>(new zh_scale_kld2008(*this));
    }
    zh_scale_kld2008()
    {
        std::string brand_name = "柯力D2008";
        if (g_scale_map[brand_name] == NULL)
        {
            g_scale_map[brand_name] = this;
        }
    }
} g_zh_scale_kld2008;

double get_current_weight(const std::string &_scale_ip, unsigned short _port, const std::string &_brand)
{
    double ret = 0.001;
    auto scale = zh_scale_if::get_scale(_brand);
    if (scale)
    {
        ret = scale->get_weight(_scale_ip, _port);
    }

    return ret;
}
void clean_scale_weight(const std::string &_scale_ip, unsigned short _port, const std::string &_brand)
{
    auto scale = zh_scale_if::get_scale(_brand);
    if (scale)
    {
        scale->clean_scale(_scale_ip, _port);
    }
}