#include "zh_id_reader.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include <iconv.h>
#include <vector>
#include "../../zh_database/zh_db_config.h"

tdf_log g_log("id_reader");
std::vector<std::string> split_string(const std::string &s, const std::string &seperator)
{
    std::vector<std::string> result;
    typedef std::string::size_type string_size;
    string_size i = 0;

    while (i != s.size())
    {
        //找到字符串中首个不等于分隔符的字母；
        int flag = 0;
        while (i != s.size() && flag == 0)
        {
            flag = 1;
            for (string_size x = 0; x < seperator.size(); ++x)
                if (s[i] == seperator[x])
                {
                    ++i;
                    flag = 0;
                    break;
                }
        }

        //找到又一个分隔符，将两个分隔符之间的字符串取出；
        flag = 0;
        string_size j = i;
        while (j != s.size() && flag == 0)
        {
            for (string_size x = 0; x < seperator.size(); ++x)
                if (s[j] == seperator[x])
                {
                    flag = 1;
                    break;
                }
            if (flag == 0)
                ++j;
        }
        if (i != j)
        {
            result.push_back(s.substr(i, j - i));
            i = j;
        }
    }
    return result;
}
std::string zh_read_id_no(const std::string &ip, unsigned short port)
{
    std::string ret;
    zh_vcom_link vl(ip, port);
    bool read_ret = false;

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
                        read_ret = true;
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

    auto all_info = split_string(ret, "|");
    if (all_info.size() > 5)
    {
        ret = all_info[5];
    }
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