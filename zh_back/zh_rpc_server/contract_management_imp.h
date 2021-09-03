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
};

#endif // _CONTRACT_MANAGEMENT_H_
