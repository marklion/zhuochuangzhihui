#include "lib/zh_hk_gate.h"
static NET_DVR_TIME dateTime2unix(const std::string &_time, int _min = 0)
{
    NET_DVR_TIME tm;
    memset(&tm, 0, sizeof(tm));
    auto timeStamp = _time;
    if (_min != 0)
    {
        auto time_sec = zh_rpc_util_get_time_by_string(_time) - _min;
        timeStamp = zh_rpc_util_get_timestring(time_sec);
    }
    sscanf(
        timeStamp.c_str(), "%d-%d-%d %d:%d:%d",
        &tm.dwYear, &tm.dwMonth, &tm.dwDay,
        &tm.dwHour, &tm.dwMinute, &tm.dwSecond);

    return tm;
}
int main(int argc, char const *argv[])
{
    if (argc < 5)
    {
        std::cout << "usage: " << argv[0] << "(pic|video) nvr_ip nvr_channel nvr_username nvr_password [begin_time] [end_time]" << std::endl;
        return -1;
    }

    std::string cmd = argv[1];
    std::string nvr_ip = argv[2];
    int nvr_channel = atoi(argv[3]);
    std::string username = argv[4];
    std::string password = argv[5];
    std::string begin_time;
    std::string end_time;
    if (argc > 6)
    {
        begin_time = argv[6];
        end_time = argv[7];
    }

    std::string path_name;
    if (cmd == "pic")
    {
        path_name = zh_hk_get_capture_picture(nvr_ip, nvr_channel, username, password);
    }
    else if (cmd == "video")
    {
        path_name = zh_hk_get_channel_video(nvr_ip, nvr_channel, dateTime2unix(begin_time), dateTime2unix(end_time), username, password);
    }

    std::cout << path_name << std::endl;

    return 0;
}
