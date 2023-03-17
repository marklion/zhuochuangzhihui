#include "../abs_device_driver.h"

extern "C"
{
#include "libtermb.h"
}

class ser_id_reader : public abs_device_driver
{
public:
    using abs_device_driver::abs_device_driver;
};

static std::string read_id(bool &_healthy)
{
    std::string ret;
    _healthy = false;
    char szWlt[2048] = {0};
    char szTxt_GB2312[500] = {0x00};
    char szTxt_UTF[1024] = {0x00};
    char szBmp[38862] = {0x00};
    char sam_id[64] = {0};
    if (1 == GetSamdId(sam_id))
    {
        if (0 != FindCardCmd())
        {
            if (0 != SelCardCmd())
            {
                if (1 == ReadCard(szTxt_GB2312, szWlt, szBmp))
                {
                    iconv_t cd = iconv_open("utf-8", "gb2312");
                    if (cd != (iconv_t)-1)
                    {
                        size_t in_len = strlen(szTxt_GB2312);
                        size_t out_len = sizeof(szTxt_UTF);
                        char *pin = szTxt_GB2312;
                        char *pout = szTxt_UTF;
                        if (-1 != iconv(cd, &pin, &in_len, &pout, &out_len))
                        {
                            ret.assign(szTxt_UTF, strlen(szTxt_UTF));
                        }
                        iconv_close(cd);
                    }
                }
            }
        }
        _healthy = true;
    }

    auto all_info = split_string(ret, "|");
    if (all_info.size() > 5)
    {
        ret = all_info[5];
        if (ret.length() > 0 && ret[ret.length() - 1] == 'X')
        {
            ret[ret.length() - 1] = 'x';
        }
    }

    return ret;
}

int main(int argc, char const *argv[])
{
    ser_id_reader sir(argc, argv, {"ip", "port"});

    zh_vcom_link com_inter(sir.get_device_arg("ip"), (unsigned short)atoi(sir.get_device_arg("port").c_str()));

    if (1 == InitComm(com_inter.get_pts().c_str()))
    {
        bool healthy = true;
        while (healthy)
        {
            usleep(200000);
            auto id = read_id(healthy);
            if (id.length() > 0)
            {
                sir.pub_id_come(id);
            }
            sir.pub_device_status(true);
        }
        CloseComm();
    }
    sir.pub_device_status(false);
    sleep(15);

    return 0;
}
