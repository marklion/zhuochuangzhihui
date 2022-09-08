#if !defined(_ZH_LED_PLAN_CALL_H_)
#define _ZH_LED_PLAN_CALL_H_

#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../../zh_pub/zh_cpp_pub/Base64.h"
#include "../../../zh_back/zh_tdf/tdf_include.h"

void zh_led_plan_call_add_plan_vehicle(const std::string &_vehicle);
void zh_led_plan_call_add_call_vehicle(const std::string &_vehicle);
void zh_led_plan_call_del_plan_vehicle(const std::string &_vehicle);
void zh_led_plan_call_del_call_vehicle(const std::string &_vehicle);
bool zh_led_plan_call_show();


#endif // _ZH_LED_PLAN_CALL_H_
