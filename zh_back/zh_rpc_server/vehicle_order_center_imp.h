#if !defined(_VEHICLE_ORDER_CENTER_IMP_H_)
#define _VEHICLE_ORDER_CENTER_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include "system_management_imp.h"
#include "../zh_tdf/tdf_include.h"
#include "../zh_database/zh_db_config.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"


class scale_sm_vehicle_come:public tdf_state_machine_state {
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name() {
        return "判定";
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
    void clear()
    {
        vehicle_number.clear();
        id_no.clear();
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
    std::string pass_permit(const std::string &_vehicle_number, const std::string &_id_no, const std::string &_qr_code);
};
class scale_state_machine : public tdf_state_machine {
public:
    tdf_log m_log;
    device_scale_config bound_scale;
    std::string bound_vehicle_number;
    std::string enter_cam_ip;
    std::string exit_cam_ip;
    int timer_fd = -1;
    std::vector<double> continue_weight;
    std::vector<int> id_read_timer;
    tdf_log &get_log() {
        return m_log;
    }
    scale_gate_trigger_param entry_param;
    scale_gate_trigger_param exit_param;
    bool trigger_switch = true;
    gate_ctrl_policy ctrl_policy;
    void open_trigger_switch();
    void proc_trigger_id_read(const std::string &_id_no, const std::string &_id_reader_ip);
    void proc_trigger_vehicle(const std::string &_vehicle_number, const std::string &_road_ip);
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
    void print_weight_ticket();
};

class gate_sm_vehicle_come:public tdf_state_machine_state {
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
class gate_state_machine:public tdf_state_machine {
public:
    tdf_log m_log;
    std::string road_ip;
    std::string id_reader_ip;
    int id_reader_timer = -1;
    bool is_entry = false;
    tdf_log &get_log() {
        return m_log;
    }
    scale_gate_trigger_param param;
    gate_ctrl_policy ctrl_policy;
    gate_state_machine(const std::string &_road_way, const std::string &_id_reader_ip, bool _is_entry);
    virtual ~gate_state_machine();
    void clean_bound_info();
    void open_door();
    void gate_cast_accept();
    void gate_cast_reject();
    bool should_open();
    void record_vehicle_pass();
    void proc_trigger_id_no(const std::string &_id_no);
    void proc_trigger_vehicle_number(const std::string &_vehicle_number);
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
            for (auto &itr:dc.scale)
            {
                ssm_map[itr.name] = std::make_shared<scale_state_machine>(itr);
            }
            for (auto &itr:dc.gate)
            {
                gsm_map[itr.entry_config.cam_ip] = std::make_shared<gate_state_machine>(itr.entry_config.cam_ip, itr.entry_id_reader_ip, true);
                gsm_map[itr.exit_config.cam_ip] = std::make_shared<gate_state_machine>(itr.exit_config.cam_ip, itr.exit_id_reader_ip, false);
            }
        }

        return m_inst;
    }
    virtual void get_order_by_anchor(std::vector<vehicle_order_info> &_return, const std::string &ssid, const int64_t anchor);
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
    virtual bool driver_check_in(const int64_t order_id, const bool is_cancel);
    virtual void driver_get_order(vehicle_order_detail &_return, const std::string &order_number);
    virtual bool call_vehicle(const std::string &ssid, const int64_t order_id, const bool is_cancel);
    virtual void get_registered_vehicle(std::vector<vehicle_order_detail> &_return, const std::string &ssid);
    virtual bool manual_set_p_weight(const std::string &ssid, const int64_t order_id, const double weight);
    virtual bool manual_set_m_weight(const std::string &ssid, const int64_t order_id, const double weight);
    virtual bool manual_close(const std::string &ssid, const int64_t order_id);
};

#endif // _VEHICLE_ORDER_CENTER_IMP_H_
