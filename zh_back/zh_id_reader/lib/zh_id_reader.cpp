#include "zh_id_reader.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include <iconv.h>

zh_log g_log("id_reader");

std::string read_id_no(const std::string &ip, unsigned short port)
{
    std::string ret;
    zh_vcom_link vl(ip, port);

    if (1 == InitComm(vl.get_pts().c_str()))
    {
        char szWlt[2048] = {0};
        char szTxt_GB2312[500] = {0x00};
        char szTxt_UTF[1024] = {0x00};
        char szBmp[38862] = {0x00};
        char sam_id[64] = {0};
        if (1 == GetSamdId(sam_id))
        {
            g_log.log("sam id: %s", sam_id);
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
                            else
                            {
                                g_log.err("iconv convert failed: %s", strerror(errno));
                            }
                            iconv_close(cd);
                        }
                        else
                        {
                            g_log.err("iconv open failed: %s", strerror(errno));
                        }
                    }
                    else
                    {
                        g_log.err("read card failed");
                    }
                }
                else
                {
                    g_log.err("select card failed");
                }
            }
            else
            {
                g_log.err("find card failed");
            }
        }
        else
        {
            g_log.err("get samid failed");
        }
        CloseComm();
    }
    else
    {
        g_log.err("open id reader failed");
    }

    return ret;
}