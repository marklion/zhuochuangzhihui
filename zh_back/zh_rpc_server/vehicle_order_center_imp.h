#if !defined(_VEHICLE_ORDER_CENTER_IMP_H_)
#define _VEHICLE_ORDER_CENTER_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include "system_management_imp.h"
#include "../zh_tdf/tdf_include.h"
#include "../zh_database/zh_db_config.h"

class vehicle_sm_init:public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name()
    {
        return "初始态";
    }
};
class vehicle_sm_before_enter_gate : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name()
    {
        return "进门前";
    }
};
class vehicle_sm_before_scale : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name()
    {
        return "上磅前";
    }
};
class vehicle_sm_before_exit_gate : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name()
    {
        return "出门前";
    }
};
class vehicle_sm_insite : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name()
    {
        return "场内";
    }
};
class vehicle_sm_scale : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name()
    {
        return "称重";
    }
};
class vehicle_sm_end : public tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm);
    virtual void do_action(tdf_state_machine &_sm);
    virtual void after_enter(tdf_state_machine &_sm);
    virtual void before_leave(tdf_state_machine &_sm);
    virtual std::string state_name()
    {
        return "结束";
    }
};
class vehicle_state_machine : public tdf_state_machine
{
public:
    static std::map<std::string, std::shared_ptr<vehicle_state_machine>> all_sm;
    std::vector<int> all_timer_handler;
    std::string plateNo;
    int sm_type = 0;
    tdf_log m_log;
    std::string current_roadway;
    bool is_timeout = false;
    std::vector<double> continue_weight;
    int id_reader_timer_handle = -1;
    int scale_timer = -1;
    std::string id_no;
    void proc_roadway_trigger(const std::string &_roadway);
    bool scale_trigger();
    bool enter_trigger();
    bool exit_trigger();
    void close_all_timer();
    bool timeout();
    bool scale_stable();
    void close_id_read();
    void clean_roadway();
    void open_door();
    void close_scale();
    void record_enter();
    void record_exit();
    void enable_id_read();
    void open_enter_timer();
    void open_exit_timer();
    void change_roadway();
    void scale_zero();
    void enable_scale();
    void record_close();
    virtual bool can_scale() = 0;
    virtual bool can_exit() = 0;
    virtual bool finish_scale() = 0;
    virtual bool no_need_gate() = 0;
    virtual bool can_enter() = 0;
    virtual void print_weight() = 0;
    virtual void record_weight() = 0;
    virtual void enter_scale_cast() = 0;
    virtual void enter_gate_cast() = 0;
    virtual void exit_gate_cast() = 0;

    vehicle_state_machine(const std::string &_plateNo, int _type) : plateNo(_plateNo), sm_type(_type), m_log(_plateNo + "sm" + std::to_string(_type))
    {
        m_cur_state.reset(new vehicle_sm_init());
        m_log.log("create");
    }
    virtual ~vehicle_state_machine()
    {
        close_all_timer();
        m_log.log("destroy");
    }
    static vehicle_state_machine &fetch_sm(const std::string &_plateNo, int _type);
    static void delete_sm(const std::string &_plateNo);
};

class no_gate_vehicle_sm : public vehicle_state_machine
{
public:
    no_gate_vehicle_sm(const std::string &_plateNo, int _type) : vehicle_state_machine(_plateNo, _type) {}
    virtual bool can_scale();
    virtual bool can_exit();
    virtual bool finish_scale();
    virtual bool no_need_gate();
    virtual bool can_enter();
    virtual void print_weight();
    virtual void record_weight();
    virtual void enter_scale_cast();
    virtual void enter_gate_cast();
    virtual void exit_gate_cast();
};
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
    virtual void get_order_by_anchor(std::vector<vehicle_order_info> &_return, const std::string &ssid, const int64_t anchor);
    virtual void get_gate_info(gate_relate_info &_return, const std::string &ssid, const int64_t order_id);
    virtual void get_weight_info(weight_relate_info &_return, const std::string &ssid, const int64_t order_id);
    virtual bool create_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
    virtual bool confirm_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
    virtual bool cancel_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order);
};

#endif // _VEHICLE_ORDER_CENTER_IMP_H_
