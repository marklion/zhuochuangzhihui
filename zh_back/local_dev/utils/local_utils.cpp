#include "local_utils.h"
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
int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}
//UNICODE码转为GB2312码
int u2g(char *inbuf, int inlen, char *outbuf, int outlen)
{
    return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}
std::string utf2gbk(const std::string &_gbk)
{
    char in_buff[9600] = {0};
    char out_buff[9600] = {0};
    strcpy(in_buff, _gbk.c_str());
    u2g(in_buff, strlen(in_buff), out_buff, sizeof(out_buff));
    return std::string(out_buff);
}
int connect_to_device_tcp_server(const std::string &_ip, unsigned short _port)
{
    int ret = -1;
    sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(_port),
        .sin_addr = {.s_addr = inet_addr(_ip.c_str())},
    };
    auto socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd >= 0)
    {
        if (0 == connect(socket_fd, (sockaddr *)&server_addr, sizeof(server_addr)))
        {
            ret = socket_fd;
        }
        else
        {
            perror("failed to connect server");
            close(socket_fd);
        }
    }
    else
    {
        perror("failed to open socket fd");
    }
    return ret;
}
time_t util_get_time_by_string(const std::string &_time_string)
{
    const char *cha = _time_string.data();                                    // 将string转换成char*。
    tm tm_ = {0};                                                             // 定义tm结构体。
    int year, month, day, hour, min, sec;                                     // 定义时间的各个int临时变量。
    sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec); // 将string存储的日期时间，转换为int临时变量。
    tm_.tm_min = min;                                                         // 时。
    tm_.tm_sec = sec;                                                         // 时。
    tm_.tm_year = year - 1900;                                                // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
    tm_.tm_mon = month - 1;                                                   // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
    tm_.tm_mday = day;                                                        // 日。
    tm_.tm_hour = hour;                                                       // 时。
    tm_.tm_isdst = 0;                                                         // 非夏令时。
    time_t t_ = mktime(&tm_);                                                 // 将tm结构体转换成time_t格式。
    return t_;                                                                // 返回值。
}
std::string local_dev_get_timestring(time_t _time)
{
    time_t cur_time = _time;

    auto st_time = localtime(&cur_time);
    char buff[512] = "";

    sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", st_time->tm_year + 1900, st_time->tm_mon + 1, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);

    return std::string(buff);
}
std::string local_dev_get_datestring(time_t _time)
{
    auto date_time = local_dev_get_timestring(_time);
    return date_time.substr(0, 10);
}