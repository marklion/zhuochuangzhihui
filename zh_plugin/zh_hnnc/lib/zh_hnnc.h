#if !defined(_ZH_HNNC_H_)
#define _ZH_HNNC_H_

#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../../zh_pub/zh_cpp_pub/Base64.h"
#include "../../../zh_back/zh_tdf/tdf_include.h"
bool ZH_HNNC_fetch_cor_info();
bool ZH_HNNC_fetch_order_info(const std::string &_cust_code);
bool ZH_HNNC_fetch_customer_info();
bool ZH_HNNC_fetch_supplier_info();
bool ZH_HNNC_fetch_supply_order_info(const std::string &_supplier_code);
bool ZH_HNNC_valid_balance(const std::string &cust_code, const std::string &inv_code, int vehicle_count);
bool ZH_HNNC_push_req(const std::string &_req_json_string);
bool ZH_HNNC_valid_remain(const std::string &_cust_code, const std::string &_inv_code, int vehicle_count);


#endif // _ZH_HNNC_H_
