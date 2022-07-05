#if !defined(_ZH_MEIYITONG_H_)
#define _ZH_MEIYITONG_H_

#include "httplib.h"
#include <string>
#include <list>
#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../../zh_pub/zh_cpp_pub/Base64.h"
#include "../../../zh_back/zh_tdf/tdf_include.h"

bool ZH_MEIYITONG_get_dest_addr();
bool ZH_MEIYITONG_get_product_info();
bool ZH_MEIYITONG_check_in(const std::string &_order_number, const std::string &_customer_name, const std::string &_product_name, const std::string &_vehicle_number);
bool ZH_MEIYITONG_post_p_weight(const std::string &_orer_number, const std::string &_vehicle_number, const std::string &_p_weight);
bool ZH_MEIYITONG_post_m_weight(const std::string &_req);
bool ZH_MEIYITONG_stuff_need_process(const std::string &_name);

#endif // _ZH_MEIYITONG_H_
