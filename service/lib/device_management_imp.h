#if !defined(_DEVICE_MANAGEMENT_H_)
#define _DEVICE_MANAGEMENT_H_

#include "../../base/include.h"
class abs_state_machine;
class abs_sm_state
{
public:
    virtual void before_enter(abs_state_machine &_sm) {}
    virtual void after_exit(abs_state_machine &_sm) {}
    virtual std::unique_ptr<abs_sm_state> proc_event(abs_state_machine &_sm)
    {
        return std::unique_ptr<abs_sm_state>();
    }
};
class abs_state_machine
{
    std::unique_ptr<abs_sm_state> m_cur_state;

public:
    abs_state_machine(std::unique_ptr<abs_sm_state> _init_state) : m_cur_state(_init_state.release())
    {
    }
    void trigger_sm()
    {
        while (1)
        {
            auto next_state = m_cur_state->proc_event(*this);
            if (next_state)
            {
                m_cur_state->after_exit(*this);
                m_cur_state.reset(next_state.release());
                m_cur_state->before_enter(*this);
            }
            else
            {
                break;
            }
        }
    }
    void init_sm()
    {
        m_cur_state->before_enter(*this);
    }
    virtual ~abs_state_machine() {}
};

class gate_state_init : public abs_sm_state
{
    virtual void before_enter(abs_state_machine &_sm);
    virtual void after_exit(abs_state_machine &_sm);
    virtual std::unique_ptr<abs_sm_state> proc_event(abs_state_machine &_sm);
};

class gate_sm : public abs_state_machine
{
public:
    int64_t set_id = 0;
    std::string pass_plate_number;
    int64_t trigger_device_id = 0;
    std::string order_number;
    gate_sm(int64_t _set_id);
};
class device_management_handler : public device_managementIf
{
    std::map<int64_t, std::shared_ptr<abs_state_machine>> m_sm_map;
    pthread_mutex_t map_lock;
public:
    device_management_handler();
    virtual void init_all_set();
    virtual bool device_ctrl(const int64_t device_id, const bool start);
    virtual bool device_is_started(const int64_t device_id);
    virtual void gate_ctrl(const int64_t gate_id, const bool is_open);
    virtual void led_display(const int64_t led_id, const std::vector<std::string> &content);
    virtual void speaker_cast(const int64_t speaker_id, const std::string &content);
    virtual double last_scale_read(const int64_t scale_id);
    virtual void last_id_read(std::string &_return, const int64_t id_reader_id);
    virtual void last_qr_read(std::string &_return, const int64_t qr_reader_id);
    virtual void last_plate_read(std::string &_return, const int64_t plate_cam_id);
    virtual void cap_picture_slow(std::string &_return, const int64_t cam_id);
    virtual void video_record_slow(std::string &_return, const int64_t cam_id, const std::string &begin_date, const std::string &end_date);
    virtual void push_scale_read(const int64_t scale_id, const double weight);
    virtual void push_id_read(const int64_t id_id, const std::string &id_number);
    virtual void push_qr_read(const int64_t qr_id, const std::string &qr_content);
    virtual void push_plate_read(const int64_t plate_cam_id, const std::string &plate_no);
    void start_device_no_exp(int64_t id);
    void sm_init_add(std::shared_ptr<abs_state_machine> _sm, int64_t sm_id);
    void sm_trigger(int64_t sm_id, std::function<bool (abs_state_machine &_sm)> update_func);
    static int64_t get_same_side_device(int64_t _input_id, const std::string &_type);
};

std::unique_ptr<device_gate_set> get_gate_config_by_id(int64_t _id);
std::unique_ptr<device_scale_set> get_scale_config_by_id(int64_t _id);

#endif // _DEVICE_MANAGEMENT_H_
