#include "contract_management_imp.h"
#include "zh_rpc_util.h"

contract_management_handler *contract_management_handler::m_inst = nullptr;

bool contract_is_dup(const contract_info &contract)
{
    bool ret = false;

    auto exist_record = sqlite_orm::search_record<zh_sql_contract>("name == '%s' AND is_sale == %ld AND PRI_ID != %ld", contract.name.c_str(), contract.is_sale?1:0, contract.id);
    if (exist_record)
    {
        ret = true;
    }

    return ret;
}

bool contract_management_handler::add_contract(const std::string &ssid, const contract_info &contract)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    if (contract_is_dup(contract))
    {
        ZH_RETURN_DUP_CONTRACT();
    }

    zh_sql_contract tmp;
    tmp.name = contract.name;
    tmp.date = zh_rpc_util_get_timestring();
    if (contract.attachment.length() > 0) {
        zh_sql_file attach_file;
        attach_file.save_file(contract.attachment.substr(0, contract.attachment.find_last_of('.')), contract.name + "-" + tmp.date + contract.attachment.substr(contract.attachment.find_last_of('.'), contract.attachment.length()));
        attach_file.insert_record();
        tmp.set_parent(attach_file, "attachment");
    }
    tmp.code = contract.code;
    tmp.is_sale = contract.is_sale ? 1 : 0;

    ret = tmp.insert_record();

    return ret;
}
bool contract_management_handler::del_contract(const std::string &ssid, const int64_t contract_id)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_contract>(contract_id);
    if (!exist_record)
    {
        ZH_RETURN_NO_CONTRACT();
    }

    exist_record->remove_record();
    ret = true;

    return ret;
}
bool contract_management_handler::update_contract(const std::string &ssid, const contract_info &contract)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_contract>(contract.id);
    if (!exist_record)
    {
        ZH_RETURN_NO_CONTRACT();
    }

    if (contract_is_dup(contract))
    {
        ZH_RETURN_DUP_CONTRACT();
    }
    exist_record->date = zh_rpc_util_get_timestring();
    if (contract.attachment.length() > 0)
    {
        auto exist_attachment = exist_record->get_parent<zh_sql_file>("attachment");
        if (!exist_attachment || exist_attachment->name != contract.attachment)
        {
            zh_sql_file attach_file;
            attach_file.save_file(contract.attachment.substr(0, contract.attachment.find_last_of('.')), contract.name + "-" + exist_record->date + contract.attachment.substr(contract.attachment.find_last_of('.'), contract.attachment.length()));
            attach_file.insert_record();
            exist_record->set_parent(attach_file, "attachment");
        }
    }
    else
    {
        zh_sql_file empty;
        exist_record->set_parent(empty, "attachment");
    }
    exist_record->code = contract.code;
    exist_record->is_sale = contract.is_sale ? 1 : 0;
    exist_record->name = contract.name;

    ret = exist_record->update_record();

    return ret;
}
void contract_management_handler::get_all_contract(std::vector<contract_info> &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 2);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto all_contract = sqlite_orm::search_record_all<zh_sql_contract>();
    for (auto &itr : all_contract)
    {
        contract_info tmp;
        auto attach_file = itr.get_parent<zh_sql_file>("attachment");
        if (attach_file)
        {
            tmp.attachment = attach_file->name;
        }
        tmp.code = itr.code;
        tmp.date = itr.date;
        tmp.id = itr.get_pri_id();
        tmp.is_sale = itr.is_sale == 0 ? false : true;
        tmp.name = itr.name;
        _return.push_back(tmp);
    }
}