#if !defined(_CONTRACT_MANAGEMENT_H_)
#define _CONTRACT_MANAGEMENT_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/contract_management.h"

class contract_management_handler : public contract_managementIf
{
private:
    static contract_management_handler *m_inst;
    contract_management_handler() {}
public:
    static contract_management_handler *get_inst() {
        if (m_inst == NULL)
        {
            m_inst = new contract_management_handler();
        }
        return m_inst;
    }
    virtual bool add_contract(const std::string &ssid, const contract_info &contract);
    virtual bool del_contract(const std::string &ssid, const int64_t contract_id);
    virtual bool update_contract(const std::string &ssid, const contract_info &contract);
    virtual void get_all_contract(std::vector<contract_info> &_return, const std::string &ssid);
    virtual void get_history(std::vector<number_change_point> &_return, const std::string &ssid, const std::string &company_name, const int64_t count);
    virtual bool change_balance(const std::string &ssid, const std::string &company_name, const double new_value, const std::string &reason);
    bool internal_change_balance(const std::string &company_name, const double new_value, const std::string &reason);
    virtual void get_contract(contract_info &_return, const std::string &ssid, const std::string &company_name);
    virtual bool add_single_contract_price(const std::string &ssid, const contract_stuff_price &bound_price);
    virtual bool del_single_contract_price(const std::string &ssid, const int64_t id);
    virtual void get_all_single_contract_price(std::vector<contract_stuff_price> &_return);
    virtual void export_history(std::vector<number_change_point> &_return, const std::string &ssid, const std::string &begin_date, const std::string &end_date, const std::string &company_name);
    virtual void get_single_price_history(std::vector<number_change_point> &_return, const std::string &ssid, const int64_t single_id, const int64_t count);
};

#endif // _CONTRACT_MANAGEMENT_H_
