#if !defined(_ZH_ZYZL_H_)
#define _ZH_ZYZL_H_
#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../../zh_pub/zh_cpp_pub/Base64.h"
#include "../../../zh_back/zh_tdf/tdf_include.h"

bool ZH_ZYZL_call_vehicle(const std::string &_plate);
bool ZH_ZYZL_push_p(const std::string &_plate);
bool ZH_ZYZL_push_weight(const std::string &_plate, const std::string &_p_time, const std::string &_m_time, double p_weight, double m_weight, double j_weight);


#endif // _ZH_ZYZL_H_
