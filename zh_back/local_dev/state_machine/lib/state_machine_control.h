#if !defined(_STATE_MACHINE_CONTROL_H_)
#define _STATE_MACHINE_CONTROL_H_

#include "../../utils/local_utils.h"
#include "../../../zh_rpc_server/plugin_management_imp.h"
#include "../../../zh_rpc_server/vehicle_order_center_imp.h"
#include "../../../zh_rpc_server/contract_management_imp.h"
#include "../../../zh_rpc_server/zh_rpc_util.h"

bool change_order_status(zh_sql_vehicle_order &_order, zh_sql_order_status &_status, const zh_order_save_hook &_hook = zh_order_save_hook());
void recalcu_balance_inventory(zh_sql_vehicle_order &_vo, const std::string &_ssid = "");
void dup_one_order(zh_sql_vehicle_order &vo);
bool pri_create_order(const std::vector<vehicle_order_info> &orders, bool from_api, const std::string &ssid = "");
struct sm_config_meta{
    std::string name;
    std::string sm_type;
    bool enter_gate_is_close = false;
    bool exit_gate_is_close = false;
    std::string status_string;
    double cur_weight = 0;
};
std::list<sm_config_meta> sm_control_get_all_sm();
std::string sm_control_take_picture(const std::string _name, bool _is_enter);
std::string sm_control_get_record(const std::string _name, bool _is_enter, const std::string &_begin_date, const std::string &_end_date);
void sm_control_open_gate(const std::string &_name, bool _is_enter, bool _is_open);
void sm_control_trigger(const std::string &_name, bool _is_enter, const std::string &_plate = "");
void sm_control_reset(const std::string &_name);
void sm_control_confirm_weight(const std::string &_name);
void sm_control_led_cast(const std::string &_name, bool _is_enter, const std::string &_content);
void sm_control_print(const std::string &_name, bool _is_enter, const std::string &_content, const std::string &_qr_content);
void sm_control_set_traffic_light(const std::string &name, bool _is_enter, bool _is_green);
std::string order_ticket_content(zh_sql_vehicle_order &_order);
#endif // _STATE_MACHINE_CONTROL_H_
