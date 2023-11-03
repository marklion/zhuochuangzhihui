#include "utils.h"
#include <openssl/md5.h>

std::vector<std::string> util_split_string(const std::string &s, const std::string &seperator)
{
    std::vector<std::string> result;
    typedef std::string::size_type string_size;
    string_size i = 0;

    while (i != s.size())
    {
        // 找到字符串中首个不等于分隔符的字母；
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

        // 找到又一个分隔符，将两个分隔符之间的字符串取出；
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

std::string util_join_string(const std::vector<std::string> &_vec, const std::string &_dil)
{
    std::string ret;

    for (const auto &itr:_vec)
    {
        ret += itr + _dil;
    }

    ret = ret.substr(0, ret.length() - _dil.length());

    return ret;
}

std::string util_calcu_md5(const std::string &_input)
{
    MD5_CTX ctx;
    unsigned char dig[16];

    MD5_Init(&ctx);
    MD5_Update(&ctx, _input.data(), _input.length());
    MD5_Final(dig, &ctx);

    return util_data_hex(dig, sizeof(dig));
}

std::string util_data_hex(const unsigned char *_data, int _length)
{
    std::string ret;

    int i = 0;
    while (i < _length)
    {
        char buf[3] = {0};
        snprintf(buf, sizeof(buf), "%02X", (unsigned int)(_data[i]));
        ret += buf;
        i++;
    }

    return ret;
}
