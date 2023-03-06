#if !defined(_ZH_PLUGIN_CONF_H_)
#define _ZH_PLUGIN_CONF_H_

#include "CJsonObject.hpp"
#include "clipp.h"

neb::CJsonObject zh_plugin_conf_get_config(const char *_file_path);
void zh_plugin_conf_set_config(const char *_file_path, const std::string &_key, const std::string &_value);
void zh_plugin_conf_set_config(const char *_file_path, const std::string &_key, const neb::CJsonObject &_value);

#endif // _ZH_PLUGIN_CONF_H_
