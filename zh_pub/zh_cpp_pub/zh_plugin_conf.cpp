#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include "Base64.h"
#include "zh_plugin_conf.h"
neb::CJsonObject zh_plugin_conf_get_config(const char *_file_path)
{
    std::fstream in_file(_file_path, std::ios::in);
    std::istreambuf_iterator<char> beg(in_file), end;
    std::string config_string(beg, end);
    neb::CJsonObject config(config_string);

    return config;
}

void zh_plugin_conf_set_config(const char *_file_path, const std::string &_key, const std::string &_value)
{
    neb::CJsonObject config(zh_plugin_conf_get_config(_file_path));
    config.Delete(_key);
    config.Add(_key, _value);

    std::fstream out_file(_file_path, std::ios::out);
    out_file << config.ToString() << std::endl;
}
void zh_plugin_conf_set_config(const char *_file_path, const std::string &_key, const neb::CJsonObject &_value)
{
    neb::CJsonObject config(zh_plugin_conf_get_config(_file_path));
    config.Delete(_key);
    config.Add(_key, _value);

    std::fstream out_file(_file_path, std::ios::out);
    out_file << config.ToString() << std::endl;
}