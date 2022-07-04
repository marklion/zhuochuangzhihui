#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include "Base64.h"
#include "zh_plugin_conf.h"
#include <pthread.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

static void lock_config_file_read(const std::string &_path)
{
    auto fd = open(_path.c_str(), O_RDWR);
    if (fd >= 0)
    {
        flock(fd, LOCK_SH);
        close(fd);
    }
}
static void lock_config_file_write(const std::string &_path)
{
    auto fd = open(_path.c_str(), O_RDWR);
    if (fd >= 0)
    {
        flock(fd, LOCK_EX);
        close(fd);
    }
}
static void unlock_config_file(const std::string &_path)
{
    auto fd = open(_path.c_str(), O_RDWR);
    if (fd >= 0)
    {
        flock(fd, LOCK_UN);
        close(fd);
    }
}

neb::CJsonObject zh_plugin_conf_get_config(const char *_file_path)
{
    lock_config_file_read(_file_path);
    std::fstream in_file(_file_path, std::ios::in);
    std::istreambuf_iterator<char> beg(in_file), end;
    std::string config_string(beg, end);
    neb::CJsonObject config(config_string);
    unlock_config_file(_file_path);

    return config;
}

void zh_plugin_conf_set_config(const char *_file_path, const std::string &_key, const std::string &_value)
{
    lock_config_file_write(_file_path);
    neb::CJsonObject config(zh_plugin_conf_get_config(_file_path));
    config.Delete(_key);
    config.Add(_key, _value);

    std::fstream out_file(_file_path, std::ios::out);
    out_file << config.ToString() << std::endl;
    unlock_config_file(_file_path);
}
void zh_plugin_conf_set_config(const char *_file_path, const std::string &_key, const neb::CJsonObject &_value)
{
    lock_config_file_write(_file_path);
    neb::CJsonObject config(zh_plugin_conf_get_config(_file_path));
    config.Delete(_key);
    config.Add(_key, _value);

    std::fstream out_file(_file_path, std::ios::out);
    out_file << config.ToString() << std::endl;
    unlock_config_file(_file_path);
}