#include "zh_id_reader.h"
#include "../../zh_vcom/zh_vcom_link.h"
#include <iconv.h>
#include <vector>
#include "../../zh_database/zh_db_config.h"
#include <sys/wait.h>

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
                            }
                            iconv_close(cd);
                        }
                        else
                        {
                        }
                    }
                    else
                    {
                    }
                }
                else
                {
                }
            }
            else
            {
            }
            read_ret = true;
        }
        else
        {
        }
    }
    else
    {
    }
    CloseComm();

    auto all_info = split_string(ret, "|");
    if (all_info.size() > 5)
    {
        ret = all_info[5];
        if (ret.length() > 0 && ret[ret.length() - 1] == 'X' )
        {
            ret[ret.length() - 1] = 'x';
        }
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

zh_read_id_api::zh_read_id_api(const std::string &_ip, unsigned short _port, std::function<void(const std::string &_id)> _func) : m_ip(_ip), m_port(_port), m_func(_func)

{
    if (m_ip.length() > 0)
    {
        pthread_create(
            &id_thread, nullptr,
            [](void *_private) -> void *
            {
                tdf_log tmp_log("id_read_api");
                tmp_log.log("start id_reader thread loop");
                auto api_this = (zh_read_id_api *)(_private);
                while (!api_this->need_exit)
                {
                    int pipfd[2] = {-1};
                    pipe(pipfd);
                    auto child_pid = fork();
                    if (child_pid > 0)
                    {
                        close(pipfd[1]);
                        int ret_status = 0;
                        char buf[1024] = {0};
                        if (0 < read(pipfd[0], buf, sizeof(buf)))
                        {
                            if (buf[0] != 'c')
                            {
                                auto func = api_this->m_func;
                                func(buf);
                            }
                        }
                        close(pipfd[0]);
                        waitpid(child_pid, &ret_status, 0);
                    }
                    else
                    {
                        close(pipfd[0]);
                        auto id_read = zh_read_id_no(api_this->m_ip, api_this->m_port);
                        if (id_read.length() > 0)
                        {
                            write(pipfd[1], id_read.data(), id_read.length());
                        }
                        else
                        {
                            write(pipfd[1], "c", 1);
                        }
                        _exit(0);
                    }
                    sleep(1);
                }
                tmp_log.log("id_reader thread loop was stopped");
            },
            this);
    }
}
zh_read_id_api::~zh_read_id_api()
{
    need_exit = true;
    if (id_thread != 0)
    {
        pthread_join(id_thread, nullptr);
    }
    tdf_log tmp_log("id_read_api");
    tmp_log.log("id_read api destroy");
}