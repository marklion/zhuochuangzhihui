#if !defined(_SCALE_SM1_H_)
#define _SCALE_SM1_H_

#include "../state_machine_common.h"
class ssm_empty : public abs_state_machine_state
{
    bool need_next = false;
    virtual void proc_event_vehicle_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_vehicle_number);
    virtual void proc_event_vehicle_id_come(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_id);
    virtual void proc_event_vehicle_qr_scan(abs_state_machine &_sm, ssm_device_type _device_type, const std::string &_qr_code);
    virtual void proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close);
    virtual void after_enter(abs_state_machine &_sm);
    virtual void before_exit(abs_state_machine &_sm);
    std::unique_ptr<abs_state_machine_state> get_next(abs_state_machine &_sm);
    virtual std::string get_cur_status() { return "空闲"; };
};
class ssm_wait_vehicle_come : public abs_state_machine_state
{
    bool need_next = false;
    int cast_count_down = 5;
    virtual void proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close);
    virtual void after_enter(abs_state_machine &_sm);
    virtual void before_exit(abs_state_machine &_sm);
    virtual void proc_manual_confirm(abs_state_machine &_sm);
    std::unique_ptr<abs_state_machine_state> get_next(abs_state_machine &_sm);
    virtual std::string get_cur_status() { return "等待上磅"; };
};
class ssm_scale : public abs_state_machine_state
{
    bool finish_weight = false;
    bool need_confirm = false;
    std::vector<double> weight_list;
    int last_sec = 0;
    virtual void proc_event_cur_weight(abs_state_machine &_sm, ssm_device_type _device_type, double _weight);
    virtual void after_enter(abs_state_machine &_sm);
    virtual void before_exit(abs_state_machine &_sm);
    std::unique_ptr<abs_state_machine_state> get_next(abs_state_machine &_sm);
    virtual std::string get_cur_status() { return "正在称重"; };
};
class ssm_confirm : public abs_state_machine_state
{
    bool need_next = false;
    virtual void after_enter(abs_state_machine &_sm);
    virtual void before_exit(abs_state_machine &_sm);
    std::unique_ptr<abs_state_machine_state> get_next(abs_state_machine &_sm);
    virtual void proc_manual_confirm(abs_state_machine &_sm);
    virtual std::string get_cur_status() { return "等待确认"; };
};
class ssm_wait_vehicle_leave : public abs_state_machine_state
{
    bool gate_is_close = false;
    bool weight_is_clear = false;
    virtual void proc_event_gate_is_close(abs_state_machine &_sm, ssm_device_type _device_type, bool _is_close);
    virtual void after_enter(abs_state_machine &_sm);
    virtual void before_exit(abs_state_machine &_sm);
    virtual void proc_event_cur_weight(abs_state_machine &_sm, ssm_device_type _device_type, double _weight);
    std::unique_ptr<abs_state_machine_state> get_next(abs_state_machine &_sm);
    virtual std::string get_cur_status() { return "等待离开"; };
};

#endif // _SCALE_SM1_H_
