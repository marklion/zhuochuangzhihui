#if !defined(_VEHICLE_ORDER_CENTER_IMP_H_)
#define _VEHICLE_ORDER_CENTER_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include "system_management_imp.h"
#include "../zh_tdf/tdf_include.h"
#include "../zh_database/zh_db_config.h"

struct vehicle_gate_detail {
    std::string name;
    std::string timestamp;
};

struct vehicle_scale_detail {
    std::string name;
    std::string timestamp;
    double weight = 0;
};
class vehicle_sm_enter_scale:public tdf_state_machine_state{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_exit_gate:public tdf_state_machine_state{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_before_enter_gate:public tdf_state_machine_state{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_insite : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_do_scale : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_before_scale : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_after_scale : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_end : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_enter_gate : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_sm_before_exit_gate : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name();
};
class vehicle_state_machine : public tdf_state_machine {
public:
    vehicle_gate_detail enter_detail;
    vehicle_gate_detail exit_detail;
    vehicle_scale_detail first_detail;
    vehicle_scale_detail second_detail;
    std::string related_gate_code;
    bool leave_flag = false;
    std::vector<double> continue_weight;
    bool enter_timeout_flag = false;
    std::vector<int> all_timer_handler;
    device_scale_config cur_scale;
    std::string cur_enter_scale_gate;
    std::string cur_exit_scale_gate;
    std::string id_no;
    std::string plateNo;
    int sm_type = 0;
    tdf_log m_log;
    static std::map<std::string, std::shared_ptr<vehicle_state_machine>> all_sm;
    bool vehicle_on_scale();
    bool vehicle_pass_gate();
    bool gate_exit_near();
    bool scale_entry_near();
    bool weight_stable();
    bool vehicle_leave_scale();
    bool enter_timeout();
    virtual bool can_enter_scale() = 0;
    virtual bool cannot_enter_scale() = 0;
    virtual bool can_leave() = 0;
    virtual bool cannot_leave() = 0;
    virtual bool can_enter() = 0;
    virtual bool cannot_enter() = 0;
    void close_all_timer();
    void clean_show_info();
    void start_scale_timer();
    void start_enter_timer();
    virtual void show_before_enter_info() = 0;
    virtual void show_before_exit_info() = 0;
    virtual void vehicle_save() = 0;
    void open_door();
    void close_door();
    void show_exit_info();
    void show_enter_info();
    void print_scale_info();
    void show_scale_info();
    void record_weight();
    void record_exit_gate_code();
    void record_enter_time();
    void record_exit_time();
    void record_cur_scale();

    void proc_gate_near(const std::string &_code);
    void proc_pass_gate(const std::string &_code);
    vehicle_state_machine(const std::string &_plateNo, int _type):plateNo(_plateNo),sm_type(_type), m_log(_plateNo + "sm" + std::to_string(_type)){
        m_cur_state.reset(new vehicle_sm_before_enter_gate());
        m_log.log("create");
    }
    virtual ~vehicle_state_machine() {
        close_all_timer();
        m_log.log("destroy");
    }
    static vehicle_state_machine &fetch_sm(const std::string &_plateNo, int _type);
    static void delete_sm(const std::string &_plateNo);
};

class none_vehicle_sm:public vehicle_state_machine {
public:
    none_vehicle_sm(const std::string &_plateNo, int _type):vehicle_state_machine(_plateNo, _type){
    }
    virtual bool can_enter_scale();
    virtual bool cannot_enter_scale();
    virtual bool can_leave();
    virtual bool cannot_leave();
    virtual bool can_enter();
    virtual bool cannot_enter();
    virtual void show_before_enter_info();
    virtual void show_before_exit_info();
    virtual void vehicle_save();
};



class vehicle_order_center_handler : public vehicle_order_centerIf
{
private:

    static vehicle_order_center_handler *m_inst;
    vehicle_order_center_handler() {
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
    virtual void get_order_by_anchor(std::vector<vehicle_order_info> &_return, const std::string &ssid, const int64_t anchor);
    virtual void get_gate_info(gate_relate_info &_return, const std::string &ssid, const int64_t order_id);
    virtual void get_weight_info(weight_relate_info &_return, const std::string &ssid, const int64_t order_id);
    virtual bool create_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
    virtual bool confirm_vehicle_order(const std::string& ssid, const std::vector<vehicle_order_info> & order);
    virtual bool cancel_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
};

#endif // _VEHICLE_ORDER_CENTER_IMP_H_
