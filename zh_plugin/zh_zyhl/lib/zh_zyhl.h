#if !defined(_ZYHL_LIB_H_)
#define _ZYHL_LIB_H_
#include <string>
#include <list>
#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../../zh_pub/zh_cpp_pub/Base64.h"
#include "../../../zh_back/zh_tdf/tdf_include.h"

void fetch_plan_from_zyhl(const std::string &_date);
std::string util_get_datestring(time_t _time);
bool push_vehicle_enter(const std::string &_vehicle_number, double _xxx);
bool push_vehicle_weight(const std::string &_vehicle_number, double _weight);

std::string send_file_to_zyhl(const std::string &_id, const std::string &_path);
#endif // _ZYHL_LIB_H_
