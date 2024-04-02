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

static int lock_config_file_read(const std::string &_path)
{
    auto fd = open(_path.c_str(), O_RDWR);
    if (fd >= 0)
    {
        flock(fd, LOCK_SH);
    }
    return fd;
}
static int lock_config_file_write(const std::string &_path)
{
    auto fd = open(_path.c_str(), O_RDWR);
    if (fd >= 0)
    {
        flock(fd, LOCK_EX);
    }

    return fd;
}
static void unlock_config_file(int fd)
{
    if (fd >= 0)
    {
        flock(fd, LOCK_UN);
        close(fd);
    }
}
static neb::CJsonObject pri_get_config(const std::string &_path)
{
    std::fstream in_file(_path, std::ios::in);
    std::istreambuf_iterator<char> beg(in_file), end;
    std::string config_string(beg, end);
    neb::CJsonObject config(config_string);

    return config;
}

neb::CJsonObject zh_plugin_conf_get_config(const char *_file_path)
{
    auto fd = lock_config_file_read(_file_path);
    auto config = pri_get_config(_file_path);
    unlock_config_file(fd);

    return config;
}

void zh_plugin_conf_set_config(const char *_file_path, const std::string &_key, const std::string &_value)
{
    auto fd = lock_config_file_write(_file_path);
    neb::CJsonObject config(pri_get_config(_file_path));
    config.Delete(_key);
    config.Add(_key, _value);

    std::fstream out_file(_file_path, std::ios::out);
    out_file << config.ToString() << std::endl;
    unlock_config_file(fd);
}
void zh_plugin_conf_set_config(const char *_file_path, const std::string &_key, const neb::CJsonObject &_value)
{
    auto fd = lock_config_file_write(_file_path);
    neb::CJsonObject config(pri_get_config(_file_path));
    config.Delete(_key);
    config.Add(_key, _value);

    std::fstream out_file(_file_path, std::ios::out);
    out_file << config.ToString() << std::endl;
    unlock_config_file(fd);
}

void zh_plugin_conf_delete_config(const char *_file_path, const std::string &_key)
{
    auto fd = lock_config_file_write(_file_path);
    neb::CJsonObject config(pri_get_config(_file_path));
    config.Delete(_key);

    std::fstream out_file(_file_path, std::ios::out);
    out_file << config.ToString() << std::endl;
    unlock_config_file(fd);
}
