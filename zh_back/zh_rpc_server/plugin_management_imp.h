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
class plugin_management_handler:public plugin_managementIf{
private:
    static plugin_management_handler *m_inst;
    tdf_log m_log;
    pthread_mutex_t m_que_lock;
    pthread_cond_t que_cond = PTHREAD_COND_INITIALIZER;
    plugin_management_handler():m_log("plugin_management") {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_que_lock, &attr);
        auto installed_plugins = internel_get_installed_plugins();
        for (auto &itr:installed_plugins)
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
                    outtime.tv_sec = now.tv_sec + 5;
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
                    for (auto &itr:wait_do)
                    {
                        std::string std_out;
                        std::string std_err;
                        zh_plugin_run_plugin("proc_event -c " + itr.get_cmd() + " " + itr.order_number, itr.plugin_name, std_out, std_err );
                        if (std_err.empty())
                        {
                            finish_event(itr.plugin_name, true);
                        }
                    }
                } })
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
    virtual void get_que_by_name(std::vector<std::string> & _return, const std::string& ssid, const std::string& plugin_name);
    virtual void pop_event_from_que(const std::string& ssid, const std::string& plugin_name);
};
