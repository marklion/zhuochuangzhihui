#include "zh_scale.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include "../../zh_tdf/tdf_include.h"
#include "../../zh_database/zh_db_config.h"
#include <math.h>
#include <termios.h>

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

    for (auto itr = g_scale_map.begin(); itr != g_scale_map.end(); ++itr)
    {
        ret.push_back(itr->first);
    }

    return ret;
}
class zh_scale_test : public zh_scale_if
{
public:
    zh_scale_test()
    {
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
            zh_runtime_get_device_health()[_scale_ip] = 2;
            g_log.err("failed to read scale");
        }
        else
        {
            zh_runtime_get_device_health()[_scale_ip] = 1;
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
            zh_runtime_get_device_health()[_scale_ip] = 2;
        }
        else
        {
            zh_runtime_get_device_health()[_scale_ip] = 1;
        }
    }
    virtual std::unique_ptr<zh_scale_if> clone_self()
    {
        return std::unique_ptr<zh_scale_if>(new zh_scale_test(*this));
    }
} g_single_test_scale;

class zh_scale_kld2008_tf1 : public zh_scale_if
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
                                 for (int i = 0; i < sizeof(ret_buff) / 2; i++)
                                 {
                                     auto tmp = ret_buff[i * 2];
                                     ret_buff[i * 2] = ret_buff[i * 2 + 1];
                                     ret_buff[i * 2 + 1] = tmp;
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
            zh_runtime_get_device_health()[_scale_ip] = 2;
        }
        else
        {
            zh_runtime_get_device_health()[_scale_ip] = 1;
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
            zh_runtime_get_device_health()[_scale_ip] = 2;
        }
        else
        {
            zh_runtime_get_device_health()[_scale_ip] = 1;
        }
    }
    virtual std::unique_ptr<zh_scale_if> clone_self()
    {
        return std::unique_ptr<zh_scale_kld2008_tf1>(new zh_scale_kld2008_tf1(*this));
    }
    zh_scale_kld2008_tf1()
    {
        std::string brand_name = "柯力D2008_tf1";
        if (g_scale_map[brand_name] == NULL)
        {
            g_scale_map[brand_name] = this;
        }
    }
} g_zh_scale_kld2008_tf1;

class zh_scale_kld2008_tf0 : public zh_scale_if
{
public:
    std::string make_one_frame(const char *_data, int _length)
    {
        std::string ret;
        if (_length >= 12)
        {
            auto p_tail = _data + _length - 1;
            auto p_head = _data;
            while (*p_tail != 0x03 && p_tail != p_head)
            {
                p_tail--;
            }
            if (p_tail - p_head >= 11)
            {
                p_head = p_tail - 11;
                if (*p_head == 0x02)
                {
                    ret.assign(p_head, 12);
                }
            }
        }

        return ret;
    }
    double parse_weight(const std::string &_frame)
    {
        double ret = 0;
        if (_frame.length() == 12)
        {
            auto sign_flag = _frame[1];

            for (auto i = 0; i < 6; i++)
            {
                auto dig = _frame[2 + i];
                ret += (dig - '0') * pow(10, (5 - i));
            }
            for (auto i = 0; i < (_frame[8] - '0'); i++)
            {
                ret /= 10;
            }

            if (sign_flag == '-')
            {
                ret = 0 - ret;
            }
        }

        return ret;
    }
    virtual double get_weight(const std::string &_scale_ip, unsigned short _port)
    {
        double ret = 0;
        zh_vcom_link vl(_scale_ip, _port);
        char buff[2048] = {0};

        int fd = open(vl.get_pts().c_str(), O_RDWR);
        if (fd >= 0)
        {
            termios opt;
            tcgetattr(fd, &opt);
            opt.c_cflag |= CLOCAL | CREAD;
            opt.c_oflag = 0;
            opt.c_lflag = 0;
            opt.c_cc[VTIME] = 0;
            opt.c_cc[VMIN] = 12;
            tcsetattr(fd, TCSANOW, &opt);
            int read_len = read(fd, buff, sizeof(buff));
            if (read_len >= 0)
            {
                ret = parse_weight(make_one_frame(buff, read_len));
            }
            else
            {
                g_log.err("failed to read data from scale : %s", strerror(errno));
            }
            close(fd);
        }
        else
        {
            g_log.err("failed to open scale device: %s", strerror(errno));
        }

        return ret;
    }
    virtual void clean_scale(const std::string &_scale_ip, unsigned short _port)
    {
        return;
    }
    virtual std::unique_ptr<zh_scale_if> clone_self()
    {
        return std::unique_ptr<zh_scale_kld2008_tf0>(new zh_scale_kld2008_tf0(*this));
    }
    zh_scale_kld2008_tf0()
    {
        std::string brand_name = "柯力D2008_tf0";
        if (g_scale_map[brand_name] == NULL)
        {
            g_scale_map[brand_name] = this;
        }
    }
} g_zh_scale_kld2008_tf0;

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