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

bool add_contract_user(const contract_info &contract, zh_sql_contract &_con_in_sql)
{
    bool ret = false;
    if (!sqlite_orm::search_record<zh_sql_user_info>("phone == '%s'", contract.admin_phone.c_str()))
    {
        zh_sql_user_info ext_user;
        ext_user.name = contract.name;
        ext_user.need_change_password = 1;
        ext_user.password = contract.admin_password;
        ext_user.phone = contract.admin_phone;
        ext_user.set_parent(_con_in_sql, "belong_contract");
        auto ext_permission = sqlite_orm::search_record<zh_sql_user_permission>("key == 3");
        if (ext_permission)
        {
            ext_user.set_parent(*ext_permission, "permission");
            ret = ext_user.insert_record();
        }
    }

    return ret;
}

bool contract_management_handler::add_contract(const std::string &ssid, const contract_info &contract)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    if (contract_is_dup(contract))
    {
        ZH_RETURN_DUP_CONTRACT();
    }

    zh_sql_contract tmp;
    tmp.name = contract.name;
    tmp.date = zh_rpc_util_get_timestring();
    if (contract.attachment.length() > 0)
    {
        zh_sql_file attach_file;
        attach_file.save_file(contract.attachment.substr(0, contract.attachment.find_last_of('.')), contract.name + "-" + tmp.date + contract.attachment.substr(contract.attachment.find_last_of('.'), contract.attachment.length()));
        attach_file.insert_record();
        tmp.set_parent(attach_file, "attachment");
    }
    tmp.code = contract.code;
    tmp.is_sale = contract.is_sale ? 1 : 0;
    tmp.address = contract.company_address;

    ret = tmp.insert_record();
    if (ret && contract.admin_phone.length() > 0)
    {
        if (!add_contract_user(contract, tmp))
        {
            tmp.remove_record();
            ZH_RETURN_MSG("手机号已注册过");
        }
    }

    for (auto &itr:contract.follow_stuffs)
    {
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", itr.c_str());
        if (stuff)
        {
            zh_sql_follow_stuff tmp_follow;
            tmp_follow.set_parent(tmp, "belong_contract");
            tmp_follow.set_parent(*stuff, "belong_stuff");
            tmp_follow.insert_record();
        }
    }

    return ret;
}
bool contract_management_handler::del_contract(const std::string &ssid, const int64_t contract_id)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_contract>(contract_id);
    if (!exist_record)
    {
        ZH_RETURN_NO_CONTRACT();
    }

    auto ext_users = exist_record->get_all_children<zh_sql_user_info>("belong_contract");
    for (auto &itr : ext_users)
    {
        itr.remove_record();
    }

    exist_record->remove_record();
    auto followed_stuff = exist_record->get_all_children<zh_sql_follow_stuff>("belong_contract");
    for (auto &itr:followed_stuff)
    {
        itr.remove_record();
    }
    ret = true;

    return ret;
}
bool contract_management_handler::update_contract(const std::string &ssid, const contract_info &contract)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_contract>(contract.id);
    if (!exist_record)
    {
        ZH_RETURN_NO_CONTRACT();
    }
    if (exist_record->credit != contract.credit && !zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_CASH, false))
    {
        ZH_RETURN_NO_PRAVILIGE();
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
    exist_record->address = contract.company_address;
    exist_record->credit = contract.credit;

    auto ext_user = exist_record->get_children<zh_sql_user_info>("belong_contract");
    if (ext_user)
    {
        if (contract.admin_phone != ext_user->phone)
        {
            ext_user->remove_record();
            ext_user.reset();
        }
    }

    if (contract.admin_phone.length() > 0 && !ext_user)
    {
        if (!add_contract_user(contract, *exist_record))
        {
            ZH_RETURN_MSG("手机号已注册过");
        }
    }
    ret = exist_record->update_record();
    auto followed_stuff = exist_record->get_all_children<zh_sql_follow_stuff>("belong_contract");
    for (auto &itr:followed_stuff)
    {
        itr.remove_record();
    }
    for (auto &itr : contract.follow_stuffs)
    {
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", itr.c_str());
        if (stuff)
        {
            zh_sql_follow_stuff tmp_follow;
            tmp_follow.set_parent(*exist_record, "belong_contract");
            tmp_follow.set_parent(*stuff, "belong_stuff");
            tmp_follow.insert_record();
        }
    }
    return ret;
}
void contract_management_handler::get_all_contract(std::vector<contract_info> &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    std::string contract_query;
    auto contract = opt_user->get_parent<zh_sql_contract>("belong_contract");
    if (contract)
    {
        contract_query = "name == '" + contract->name + "'";
    }
    auto all_contract = sqlite_orm::search_record_all<zh_sql_contract>(contract_query);
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
        tmp.company_address = itr.address;
        tmp.balance = itr.balance;
        tmp.credit = itr.credit;
        auto ext_user = itr.get_children<zh_sql_user_info>("belong_contract");
        if (ext_user)
        {
            tmp.admin_phone = ext_user->phone;
        }
        auto followed_stuff = itr.get_all_children<zh_sql_follow_stuff>("belong_contract");
        for (auto &single_stuff:followed_stuff)
        {
            auto stuff = single_stuff.get_parent<zh_sql_stuff>("belong_stuff");
            if (stuff)
            {
                tmp.follow_stuffs.push_back(stuff->name);
            }
        }
        _return.push_back(tmp);
    }
}

void contract_management_handler::get_history(std::vector<number_change_point> &_return, const std::string &ssid, const std::string &company_name, const int64_t count)
{
    auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", company_name.c_str());
    if (!contract)
    {
        ZH_RETURN_NO_CONTRACT();
    }
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, true);
    if (!user)
    {
        user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
    }
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_ORDER);
    }
    auto points = contract->get_all_children<zh_sql_balance_point>("belong_contract", "PRI_ID != 0 ORDER BY PRI_ID DESC LIMIT 10 OFFSET %ld", count);
    for (auto &itr : points)
    {
        number_change_point tmp;
        tmp.change_value = itr.change_value;
        tmp.new_value = itr.new_value;
        tmp.reason = itr.reason;
        tmp.timestamp = itr.timestamp;
        _return.push_back(tmp);
    }
}

bool contract_management_handler::internal_change_balance(const std::string &company_name, const double new_value, const std::string &reason)
{
    bool ret = false;
    auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", company_name.c_str());
    if (!contract)
    {
        ZH_RETURN_NO_CONTRACT();
    }

    zh_sql_balance_point tmp;
    tmp.change_value = new_value - contract->balance;
    tmp.new_value = new_value;
    tmp.reason = reason;
    tmp.timestamp = zh_rpc_util_get_timestring();
    tmp.set_parent(*contract, "belong_contract");

    if (tmp.insert_record())
    {
        contract->balance = new_value;
        ret = contract->update_record();
    }

    return ret;
}

bool contract_management_handler::change_balance(const std::string &ssid, const std::string &company_name, const double new_value, const std::string &reason)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_CASH, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_CASH);
    }
    auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", company_name.c_str());
    if (!company)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    return internal_change_balance(company_name, company->balance + new_value, reason);
}

void contract_management_handler::get_contract(contract_info &_return, const std::string &ssid, const std::string &company_name)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true);
    auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", company_name.c_str());

    if (!contract)
    {
        ZH_RETURN_NO_CONTRACT();
    }
    if (!user)
    {
        user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
    }
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    _return.balance = contract->balance;
    _return.code = contract->code;
    _return.company_address = contract->address;
    _return.credit = contract->credit;
    _return.id = contract->get_pri_id();
    _return.is_sale = contract->is_sale;
    _return.name = contract->name;
}

bool contract_management_handler::add_single_contract_price(const std::string &ssid, const contract_stuff_price &bound_price)
{
    bool ret = false;

    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_CASH, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_CASH);
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_contract_stuff_price>(bound_price.id);
    zh_sql_single_stuff_price_point tmp;
    if (!exist_record)
    {
        exist_record.reset(new zh_sql_contract_stuff_price());
    }
    else
    {
        tmp.change_value = bound_price.price - exist_record->price;
        tmp.new_value = bound_price.price;
        tmp.reason = "";
        tmp.timestamp = zh_rpc_util_get_timestring();
        tmp.set_parent(*exist_record, "belong_price");
    }
    if (exist_record)
    {
        exist_record->customer_name = bound_price.customer_name;
        exist_record->stuff_name = bound_price.stuff_name;
        exist_record->price = bound_price.price;
        if (exist_record->get_pri_id() > 0)
        {
            ret = exist_record->update_record();
            tmp.insert_record();
        }
        else
        {
            ret = exist_record->insert_record();
        }
    }

    return ret;
}
bool contract_management_handler::del_single_contract_price(const std::string &ssid, const int64_t id)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_CASH, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_CASH);
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_contract_stuff_price>(id);
    if (exist_record)
    {
        ret = true;
        exist_record->remove_record();
    }

    return ret;
}
void contract_management_handler::get_all_single_contract_price(std::vector<contract_stuff_price> &_return)
{
    auto all_scp = sqlite_orm::search_record_all<zh_sql_contract_stuff_price>();
    for (auto &itr : all_scp)
    {
        contract_stuff_price tmp;
        tmp.customer_name = itr.customer_name;
        tmp.id = itr.get_pri_id();
        tmp.price = itr.price;
        tmp.stuff_name = itr.stuff_name;
        _return.push_back(tmp);
    }
}

void contract_management_handler::export_history(std::vector<number_change_point> &_return, const std::string &ssid, const std::string &begin_date, const std::string &end_date, const std::string &company_name)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, true);
    auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", company_name.c_str());
    if (!user || !company)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_ORDER);
    }
    auto history = company->get_all_children<zh_sql_balance_point>("belong_contract", "datetime(timestamp) >= datetime('%s') AND datetime(timestamp) < datetime('%s')", begin_date.c_str(), end_date.c_str());
    for (auto &itr : history)
    {
        number_change_point tmp;
        tmp.change_value = itr.change_value;
        tmp.new_value = itr.new_value;
        tmp.reason = itr.reason;
        tmp.timestamp = itr.timestamp;
        _return.push_back(tmp);
    }
}

void contract_management_handler::get_single_price_history(std::vector<number_change_point> &_return, const std::string &ssid, const int64_t single_id, const int64_t count)
{
    auto single_price = sqlite_orm::search_record<zh_sql_contract_stuff_price>(single_id);
    if (!single_price)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_CASH, true);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_CASH);
    }
    auto points = single_price->get_all_children<zh_sql_single_stuff_price_point>("belong_price", "PRI_ID != 0 ORDER BY PRI_ID DESC LIMIT 10 OFFSET %ld", count);
    for (auto &itr : points)
    {
        number_change_point tmp;
        tmp.change_value = itr.change_value;
        tmp.new_value = itr.new_value;
        tmp.reason = itr.reason;
        tmp.timestamp = itr.timestamp;
        _return.push_back(tmp);
    }
}
