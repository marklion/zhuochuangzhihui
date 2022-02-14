#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/plugin_management.h"
#include "../zh_tdf/tdf_include.h"
class plugin_management_handler:public plugin_managementIf{
private:
    static plugin_management_handler *m_inst;
    tdf_log m_log;
    plugin_management_handler():m_log("plugin_management") {


    }
public:
    static plugin_management_handler *get_inst() {
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
};
