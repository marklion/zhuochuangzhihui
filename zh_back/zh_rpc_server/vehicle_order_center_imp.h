#if !defined(_VEHICLE_ORDER_CENTER_IMP_H_)
#define _VEHICLE_ORDER_CENTER_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include "system_management_imp.h"
#include "../zh_tdf/tdf_include.h"
#include "../zh_database/zh_db_config.h"
#include "../zh_id_reader/lib/zh_id_reader.h"
class vehicle_order_center_handler : public vehicle_order_centerIf
{
private:
    static vehicle_order_center_handler *m_inst;
    vehicle_order_center_handler()
    {
    }

public:
    static vehicle_order_center_handler *get_inst()
    {
        if (m_inst == NULL)
        {
            m_inst = new vehicle_order_center_handler();
        }

        return m_inst;
    }
    virtual void get_order_by_anchor(std::vector<vehicle_order_detail> &_return, const std::string &ssid, const int64_t anchor, const std::string &status_name, const std::string &enter_date);
    virtual void get_gate_info(gate_relate_info &_return, const std::string &ssid, const int64_t order_id);
    virtual void get_weight_info(weight_relate_info &_return, const std::string &ssid, const int64_t order_id);
    virtual bool create_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order, const bool from_api);
    virtual bool confirm_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
    virtual bool cancel_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order, const bool from_api);
    virtual void get_order_detail(vehicle_order_detail &_return, const std::string &ssid, const std::string &order_number);
    virtual bool confirm_order_deliver(const std::string& ssid, const std::string& order_number, const bool confirmed, const std::string& inv_name);
    virtual bool update_vehicle_order(const std::string &ssid, const vehicle_order_info &order);
    virtual bool driver_check_in(const int64_t order_id, const bool is_cancel, const std::string& driver_id, const std::string& max_load);
    virtual void driver_get_order(vehicle_order_detail &_return, const std::string &order_number, const std::string &plate);
    virtual bool call_vehicle(const std::string &ssid, const int64_t order_id, const bool is_cancel);
    virtual void get_registered_vehicle(std::vector<vehicle_order_detail> &_return, const std::string &ssid);
    virtual bool manual_set_p_weight(const std::string &ssid, const int64_t order_id, const double weight);
    virtual bool manual_set_m_weight(const std::string &ssid, const int64_t order_id, const double weight);
    virtual bool manual_close(const std::string &ssid, const int64_t order_id);
    virtual void get_order_statistics(vehicle_order_statistics &_return, const std::string &ssid);
    virtual bool upload_enter_weight_attachment(const int64_t order_id, const std::string &attachment, const double enter_weight);
    virtual bool print_weight_ticket(const std::string &ssid, const int64_t order_id, const std::string &printer_ip);
    virtual void check_price_balance(std::string &_return, const std::string &ssid, const std::vector<vehicle_order_info> &order);
    virtual bool create_driver_self_order(const driver_self_order &order);
    virtual bool confirm_driver_self_order(const std::string &ssid, const int64_t order_id, const bool continue_order);
    virtual bool cancel_driver_self_order(const std::string &ssid, const int64_t order_id);
    virtual void get_all_self_order(std::vector<driver_self_order> &_return, const std::string &ssid);
    virtual void get_self_order_by_phone(driver_self_order &_return, const std::string &driver_phone);
    virtual bool record_order_source_dest(const int64_t order_id, const std::string &source_dest_name);
    virtual void get_driver_opt_history(vehicle_order_info &_return, const std::string &driver_phone);
    virtual void get_white_record_info(std::vector<white_record_info> &_return, const std::string &ssid, const int64_t begin_date, const int64_t end_date);
    bool pri_call_vehicle(const int64_t order_id, const bool is_cancel, const std::string &_user_name = "");
    virtual void driver_get_last_30_order_number(std::vector<vehicle_order_detail> &_return, const std::string &driver_phone);
    virtual void export_order_by_condition(std::vector<vehicle_order_detail> &_return, const std::string &ssid, const std::string &begin_date, const std::string &end_date, const std::string &company_name, const std::string &stuff_name);
    virtual void go_through_plugin_que(std::vector<std::string> &_return, const std::string &ssid);
    virtual void cancel_plugin_que(const std::string &ssid);
    virtual void record_white_vehicle_stuff(std::string &_return, const std::string &vehicle_number, const std::string &stuff_name);
    virtual void get_white_vehicle_stuff(std::string &_return, const std::string &vehicle_number);
    void execute_auto_call(const std::string &_stuff_name);
    virtual bool set_seal_no(const std::string &ssid, const std::string &order_number, const std::string &seal_no);
    virtual bool manual_push_nc(const std::string &order_number, const bool is_p);
    virtual void get_today_xy_vehicle(std::vector<vehicle_order_detail> &_return);
    virtual bool clear_vehicle_xy(const std::string &order_number);
};


#endif // _VEHICLE_ORDER_CENTER_IMP_H_
