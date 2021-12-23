#if !defined(_STUFF_MANAGEMENT_H_)
#define _STUFF_MANAGEMENT_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/stuff_management.h"

class stuff_management_handler : public stuff_managementIf
{
private:
    static stuff_management_handler *m_inst;
    stuff_management_handler() {}
    std::string last_active_stuff;
public:
    static stuff_management_handler *get_inst() {
        if (nullptr == m_inst) {
            m_inst = new stuff_management_handler();
        }

        return m_inst;
    }
    virtual bool add_stuff(const std::string &ssid, const stuff_info &stuff);
    virtual bool update_stuff(const std::string &ssid, const stuff_info &stuff);
    virtual bool del_stuff(const std::string &ssid, const int64_t id);
    virtual void get_all_stuff(std::vector<stuff_info> &_return, const std::string &ssid);
    virtual void get_change_points_for_range(std::vector<stuff_history> &_return, const std::string &ssid, const std::string &start_date, const std::string &end_date);
    virtual void get_last_active(std::string &_return, const std::string &ssid);
};

#endif // _STUFF_MANAGEMENT_H_
