#include "../zh_tdf/tdf_include.h"
#include <fstream>
#include "../zh_database/CJsonObject.hpp"

class zh_device_mock {
    unsigned short m_port = 0;
    std::string m_mock_data;
    tdf_log m_log;
    int m_req_len = 0;
    std::string recv_buff;
    static std::map<std::string, zh_device_mock> g_mock_map;
public:
    zh_device_mock() {}
    static void install_mock(zh_device_mock &_mock) {
        tdf_main::get_inst().open_listen(
            _mock.m_port,
            [=](const std::string &_chrct)
            {
                g_mock_map[_chrct] = _mock;
            },
            [=](const std::string &_chrct)
            {
                g_mock_map.erase(_chrct);
            },
            [=](const std::string &_chrct, const std::string &_data)
            {
                g_mock_map[_chrct].recv_buff.append(_data);
                if (g_mock_map[_chrct].recv_buff.size() >= _mock.m_req_len || _mock.m_req_len == 0)
                {
                    _mock.m_log.log("recv data");
                    _mock.m_log.log_package(g_mock_map[_chrct].recv_buff.data(), _mock.m_req_len);
                    char *out_buf = (char *)calloc(1UL, _mock.m_mock_data.length() / 2);
                    for (int i = 0; i < _mock.m_mock_data.length() / 2; i++)
                    {
                        sscanf(_mock.m_mock_data.c_str() + i * 2, "%02x", out_buf + i);
                    }
                    tdf_main::get_inst().send_data(_chrct, std::string(out_buf, _mock.m_mock_data.size() / 2));
                    _mock.m_log.log("send data");
                    _mock.m_log.log_package(out_buf, _mock.m_mock_data.size() / 2);
                    free(out_buf);
                    g_mock_map[_chrct].recv_buff.erase(0, _mock.m_req_len);
                }
            });
    }
    zh_device_mock(unsigned short _port, const std::string &_mock_data, int _req_len) : m_port(_port), m_mock_data(_mock_data), m_log("device_mock:" + std::to_string(_port)), m_req_len(_req_len) {}
};

std::map<std::string, zh_device_mock> zh_device_mock::g_mock_map;
int main(int argc, char const *argv[])
{
    std::ifstream config_file(argv[1], std::ios::in);
    std::istreambuf_iterator<char> beg(config_file), end;
    std::string config_string(beg, end);
    neb::CJsonObject config(config_string);

    for (int i = 0; i < config.GetArraySize(); i++)
    {
        zh_device_mock tmp(atoi(config[i]("port").c_str()), config[i]("data"), atoi(config[i]("req_len").c_str()));
        zh_device_mock::install_mock(tmp);
    }

    tdf_main::get_inst().run();

    return 0;
}
