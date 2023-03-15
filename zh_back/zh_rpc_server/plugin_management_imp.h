
#if !defined(_PLUGIN_MANGEMENT_IMP_H_)
#define _PLUGIN_MANGEMENT_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/plugin_management.h"
#include "../zh_tdf/tdf_include.h"
struct plugin_event_info{
    std::string order_number;
    enum event_type {
        create_order,confirm_order,vehicle_come_in,vehicle_p_weight,vehicle_m_weight,vehicle_leave,order_close,driver_register,call_driver,driver_unregister,cancel_call_driver
    } type;
    std::string plugin_name;
    std::string get_cmd() const {
        std::string ret;
        switch (type) {
            case create_order:
                ret = "create_order";
            break;
            case confirm_order:
                ret = "confirm_order";
            break;
            case vehicle_come_in:
                ret = "vehicle_come_in";
            break;
            case vehicle_p_weight:
                ret = "vehicle_p_weight";
                break;
            case vehicle_m_weight:
                ret = "vehicle_m_weight";
            break;
            case vehicle_leave:
                ret = "vehicle_leave";
            break;
            case order_close:
                ret = "order_close";
            break;
            case driver_register:
                ret = "driver_register";
            break;
            case call_driver:
                ret = "call_driver";
            break;
            case driver_unregister:
                ret = "driver_unregister";
            break;
            case cancel_call_driver:
                ret = "cancel_call_driver";
            break;
        }

        return ret;
    }
};
class plugin_management_handler : public plugin_managementIf
{
private:
    static plugin_management_handler *m_inst;
    tdf_log m_log;
    pthread_mutex_t m_que_lock;
    pthread_cond_t que_cond = PTHREAD_COND_INITIALIZER;
    plugin_management_handler() : m_log("plugin_management")
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_que_lock, &attr);
        auto installed_plugins = internel_get_installed_plugins();
        for (auto &itr : installed_plugins)
        {
            subscribe_event(itr);
        }
        std::thread(
            [&]()
            {
                while (1)
                {
                    pthread_mutex_lock(&m_que_lock);
                    timeval now;
                    timespec outtime;
                    gettimeofday(&now, NULL);
                    outtime.tv_sec = now.tv_sec + 120;
                    outtime.tv_nsec = now.tv_usec * 1000;
                    pthread_cond_timedwait(&que_cond, &m_que_lock, &outtime);
                    std::list<plugin_event_info> wait_do;
                    for (auto itr = event_deliver_map.begin(); itr != event_deliver_map.end(); itr++)
                    {
                        if (itr->second.size() > 0)
                        {
                            auto &tmp = itr->second.front();
                            tmp.plugin_name = itr->first;
                            wait_do.push_back(tmp);
                        }
                    }
                    pthread_mutex_unlock(&m_que_lock);
                    for (auto &itr : wait_do)
                    {
                        struct tmp_aw_param
                        {
                            plugin_management_handler *self_this = nullptr;
                            std::string cmd;
                            std::string order_number;
                            std::string plugin_name;
                        } *p_tmp_aw = new tmp_aw_param();
                        p_tmp_aw->cmd = itr.get_cmd();
                        p_tmp_aw->self_this = this;
                        p_tmp_aw->order_number = itr.order_number;
                        p_tmp_aw->plugin_name = itr.plugin_name;
                        tdf_main::get_inst().Async_to_workthread(
                            [](void *_private, const std::string &_chrct)
                            {
                                auto self_this = (tmp_aw_param *)(_private);
                                std::string std_out;
                                std::string std_err;
                                self_this->self_this->zh_plugin_run_plugin("proc_event -c " + self_this->cmd + " " + self_this->order_number, self_this->plugin_name, std_out, std_err);
                                if (std_err.empty())
                                {
                                    self_this->self_this->finish_event(self_this->plugin_name, true);
                                }
                                delete self_this;
                            },
                            p_tmp_aw, "");
                    }
                }
            })
            .detach();
    }
    std::map<std::string, std::list<plugin_event_info>> event_deliver_map;

public:
    static plugin_management_handler *get_inst()
    {
        if (m_inst == nullptr)
        {
            m_inst = new plugin_management_handler();
        }
        return m_inst;
    }
    virtual void run_plugin_cmd(std::string &_return, const std::string &ssid, const std::string &plugin_name, const std::string &cmd);
    virtual bool install_plugin(const std::string &ssid, const std::string &plugin_name, const std::string &file_name);
    virtual void uninstall_plugin(const std::string &ssid, const std::string &plugin_name);
    virtual void get_installed_plugins(std::vector<std::string> &_return, const std::string &ssid);
    void zh_plugin_run_plugin(const std::string &_cmd, const std::string &_plugin_name, std::string &_stdout_string, std::string &_stderr_string);
    std::vector<std::string> internel_get_installed_plugins();
    void deliver_event(const plugin_event_info &_event);
    bool subscribe_event(const std::string &_plugin_name);
    void unsubscribe_event(const std::string &_plugin_name);
    void finish_event(const std::string &_plugin_name, bool runing_only = false);
    virtual void get_que_by_name(std::vector<std::string> &_return, const std::string &ssid, const std::string &plugin_name);
    virtual void pop_event_from_que(const std::string &ssid, const std::string &plugin_name);
};
#endif