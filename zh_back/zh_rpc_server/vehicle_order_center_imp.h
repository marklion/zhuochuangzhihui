#if !defined(_VEHICLE_ORDER_CENTER_IMP_H_)
#define _VEHICLE_ORDER_CENTER_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include "system_management_imp.h"
#include "../zh_tdf/tdf_include.h"
#include "../zh_database/zh_db_config.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"
#include "../zh_id_reader/lib/zh_id_reader.h"


class scale_sm_vehicle_come:public tdf_state_machine_state {
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name() {
        return "空闲";
    }
};class scale_sm_scale:public tdf_state_machine_state {
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name() {
        return "称重";
    }
};class scale_sm_clean:public tdf_state_machine_state {
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name() {
        return "清理";
    }
};

struct scale_gate_trigger_param {
    std::string vehicle_number;
    std::string id_no;
    std::string qr_code;
    void clear()
    {
        vehicle_number.clear();
        id_no.clear();
        qr_code.clear();
    }
};

class gate_ctrl_policy {
public:
    bool need_id = false;
    bool need_qr = false;
    void set_policy(bool _need_id, bool _need_qr) {
        need_id = _need_id;
        need_qr = _need_qr;
    }
    bool is_gate = false;
    std::string pass_permit(const std::string &_vehicle_number, const std::string &_id_no, const std::string &_qr_code);
};
class scale_state_machine : public tdf_state_machine {
public:
    tdf_log m_log;
    device_scale_config bound_scale;
    std::string bound_vehicle_number;
    std::string enter_cam_ip;
    std::string exit_cam_ip;
    std::string last_enter_cam_ip;
    int timer_fd = -1;
    std::vector<double> continue_weight;
    std::vector<int> id_read_timer;
    tdf_log &get_log() {
        return m_log;
    }
    double fin_weight = 0;
    scale_gate_trigger_param entry_param;
    scale_gate_trigger_param exit_param;
    bool trigger_switch = true;
    gate_ctrl_policy ctrl_policy;
    zh_read_id_api entry_id_api;
    zh_read_id_api exit_id_api;
    bool manual_confirm_scale = true;
    bool lack_weight = false;
    void open_trigger_switch();
    void proc_trigger_id_read(const std::string &_id_no, const std::string &_id_reader_ip);
    void proc_trigger_vehicle(const std::string &_vehicle_number, const std::string &_road_ip);
    void proc_trigger_qr(const std::string &_qr_code, const std::string &_road_ip);
    void record_entry_exit();
    scale_state_machine(const device_scale_config &_config);
    virtual ~scale_state_machine();
    void open_enter();
    void open_exit();
    void scale_zero();
    void open_scale_timer();
    void close_timer();
    void clean_bound_info();
    bool should_open();
    bool scale_stable();
    bool scale_clear();
    void broadcast_enter_scale();
    void broadcast_leave_scale();
    void print_weight_ticket(const std::unique_ptr<zh_sql_vehicle_order> &_order);
    std::unique_ptr<zh_sql_vehicle_order> record_order();
    bool assume_stable_considering_manual(double _weight);
    void proc_manual_confirm_scale();
    void set_traffic_lights_red();
    void set_traffic_lights_green();
};

class gate_sm_vehicle_come : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name()
    {
        return "压线";
    }
};
class gate_state_machine : public tdf_state_machine
{
public:
    tdf_log m_log;
    std::string road_ip;
    std::string id_reader_ip;
    std::string qr_ip;
    int id_reader_timer = -1;
    bool is_entry = false;
    bool leave_timeout = false;
    tdf_log &get_log()
    {
        return m_log;
    }
    scale_gate_trigger_param param;
    gate_ctrl_policy ctrl_policy;
    zh_read_id_api id_api;
    gate_state_machine(const std::string &_road_way, const std::string &_id_reader_ip, const std::string &_qr_ip, bool _is_entry);
    virtual ~gate_state_machine();
    void clean_bound_info();
    void open_door();
    void gate_cast_accept();
    void gate_cast_reject();
    bool should_open();
    void record_vehicle_pass();
    void proc_trigger_id_no(const std::string &_id_no);
    void proc_trigger_vehicle_number(const std::string &_vehicle_number);
    void proc_trigger_qr_code(const std::string &_qr_code);
};
class vehicle_order_center_handler : public vehicle_order_centerIf
{
private:
    static vehicle_order_center_handler *m_inst;
    vehicle_order_center_handler()
    {
    }

public:
    static std::map<std::string, std::shared_ptr<scale_state_machine>> ssm_map;
    static std::map<std::string, std::shared_ptr<gate_state_machine>> gsm_map;
    static vehicle_order_center_handler *get_inst()
    {
        if (m_inst == NULL)
        {
            m_inst = new vehicle_order_center_handler();
            device_config dc;
            system_management_handler::get_inst()->internal_get_device_config(dc);
            for (auto &itr : dc.scale)
            {
                ssm_map[itr.name] = std::make_shared<scale_state_machine>(itr);
                ssm_map[itr.name]->ctrl_policy.set_policy(itr.need_id, itr.need_qr);
            }
            for (auto &itr : dc.gate)
            {
                gsm_map[itr.entry_config.cam_ip] = std::make_shared<gate_state_machine>(itr.entry_config.cam_ip, itr.entry_id_reader_ip, itr.entry_qr_ip, true);
                gsm_map[itr.entry_config.cam_ip]->ctrl_policy.set_policy(itr.entry_need_id, itr.entry_need_qr);
                gsm_map[itr.exit_config.cam_ip] = std::make_shared<gate_state_machine>(itr.exit_config.cam_ip, itr.exit_id_reader_ip, itr.exit_qr_ip, false);
                gsm_map[itr.exit_config.cam_ip]->ctrl_policy.set_policy(itr.exit_need_id, itr.exit_need_qr);
            }
        }

        return m_inst;
    }
    virtual void get_order_by_anchor(std::vector<vehicle_order_info> &_return, const std::string &ssid, const int64_t anchor, const std::string &status_name, const std::string &enter_date);
    virtual void get_gate_info(gate_relate_info &_return, const std::string &ssid, const int64_t order_id);
    virtual void get_weight_info(weight_relate_info &_return, const std::string &ssid, const int64_t order_id);
    virtual bool create_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
    virtual bool confirm_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
    virtual bool cancel_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
    virtual void get_order_detail(vehicle_order_detail &_return, const std::string &ssid, const std::string &order_number);
    virtual bool confirm_order_deliver(const std::string &ssid, const std::string &order_number, const bool confirmed);
    std::shared_ptr<scale_state_machine> get_scale_sm(const std::string &_name);
    std::shared_ptr<gate_state_machine> get_gate_sm(const std::string &_road_way);
    virtual bool update_vehicle_order(const std::string &ssid, const vehicle_order_info &order);
    virtual bool driver_check_in(const int64_t order_id, const bool is_cancel, const std::string& driver_id);
    virtual void driver_get_order(vehicle_order_detail &_return, const std::string &order_number);
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
    virtual bool record_white_vehicle_stuff(const std::string &vehicle_number, const std::string &stuff_name);
    virtual void get_white_vehicle_stuff(std::string &_return, const std::string &vehicle_number);
};

#endif // _VEHICLE_ORDER_CENTER_IMP_H_
