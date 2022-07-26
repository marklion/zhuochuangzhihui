#include "vehicle_management_imp.h"
#include "zh_rpc_util.h"

vehicle_management_handler *vehicle_management_handler::m_inst = nullptr;

bool vehicle_is_dup(const vehicle_info &vehicle)
{
    bool ret = false;

    auto exist_record = sqlite_orm::search_record<zh_sql_vehicle>("PRI_ID != %ld AND company_name == '%s' AND (main_vehicle_number == '%s' OR (behind_vehicle_number != '' AND behind_vehicle_number == '%s'))",vehicle.id , vehicle.company_name.c_str(), vehicle.main_vehicle_number.c_str(), vehicle.behind_vehicle_number.c_str());
    if (exist_record)
    {
        ret = true;
    }

    return ret;
}

bool vehicle_driver_is_dup(const vehicle_info &vehicle)
{
    bool ret = false;
    if (vehicle.driver_id.length() > 0)
    {
        auto exist_record = sqlite_orm::search_record<zh_sql_vehicle>("PRI_ID != %ld AND company_name == '%s' AND driver_id == '%s'", vehicle.id, vehicle.company_name.c_str(), vehicle.driver_id.c_str());
        if (exist_record)
        {
            ret = true;
        }
    }

    return ret;
}

bool vehicle_management_handler::add_vehicle(const std::string &ssid, const vehicle_info &vehicle)
{
    bool ret = false;

    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, false);
    if (!opt_user)
    {
        auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", vehicle.company_name.c_str());
        if (contract)
        {
            opt_user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
        }
    }
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    if (vehicle_is_dup(vehicle))
    {
        ZH_RETURN_DUP_VEHICLE();
    }

    if (vehicle_driver_is_dup(vehicle))
    {
        ZH_RETURN_DUP_DRIVER();
    }

    zh_sql_vehicle tmp;
    tmp.behind_vehicle_number = vehicle.behind_vehicle_number;
    tmp.company_name = vehicle.company_name;
    tmp.driver_id = vehicle.driver_id;
    tmp.driver_name = vehicle.driver_name;
    tmp.driver_phone = vehicle.driver_phone;
    tmp.group_name = vehicle.group_name;
    tmp.main_vehicle_number = vehicle.main_vehicle_number;
    tmp.max_count = vehicle.max_count;
    auto permission = opt_user->get_parent<zh_sql_user_permission>("permission");
    if (permission && permission->key != 3)
    {
        tmp.in_white_list = vehicle.in_white_list;
    }

    ret = tmp.insert_record();

    return ret;
}
bool vehicle_management_handler::update_vehicle(const std::string &ssid, const vehicle_info &vehicle)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, false);
    if (!opt_user)
    {
        auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", vehicle.company_name.c_str());
        if (contract)
        {
            opt_user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
        }
    }
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_vehicle>(vehicle.id);
    if (!exist_record)
    {
        ZH_RETURN_NO_VEHICLE();
    }

    if (vehicle_is_dup(vehicle))
    {
        ZH_RETURN_DUP_VEHICLE();
    }

    if (vehicle_driver_is_dup(vehicle))
    {
        ZH_RETURN_DUP_DRIVER();
    }

    exist_record->behind_vehicle_number = vehicle.behind_vehicle_number;
    exist_record->company_name = vehicle.company_name;
    exist_record->driver_id = vehicle.driver_id;
    exist_record->driver_name = vehicle.driver_name;
    exist_record->driver_phone = vehicle.driver_phone;
    exist_record->group_name = vehicle.group_name;
    exist_record->main_vehicle_number = vehicle.main_vehicle_number;
    exist_record->max_count = vehicle.max_count;
    auto permission = opt_user->get_parent<zh_sql_user_permission>("permission");
    if (permission && permission->key != 3)
    {
        exist_record->in_white_list = vehicle.in_white_list;
    }

    ret = exist_record->update_record();

    return ret;
}
bool vehicle_management_handler::del_vehicle(const std::string &ssid, const int64_t vehicle_id)
{
    bool ret = false;

    auto exist_record = sqlite_orm::search_record<zh_sql_vehicle>(vehicle_id);
    if (!exist_record)
    {
        ZH_RETURN_NO_VEHICLE();
    }

    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, false);
    if (!opt_user)
    {
        auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", exist_record->company_name.c_str());
        if (contract)
        {
            opt_user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
        }
    }
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    exist_record->remove_record();
    ret = true;

    return ret;
}
void vehicle_management_handler::get_all_vehicle(std::vector<vehicle_info> &_return, const std::string &ssid)
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
        contract_query = "company_name == '" + contract->name + "'";
    }
    auto all_vehicle = sqlite_orm::search_record_all<zh_sql_vehicle>(contract_query);
    for (auto &itr : all_vehicle)
    {
        vehicle_info tmp;
        tmp.behind_vehicle_number = itr.behind_vehicle_number;
        tmp.company_name = itr.company_name;
        tmp.driver_id = itr.driver_id;
        tmp.driver_name = itr.driver_name;
        tmp.driver_phone = itr.driver_phone;
        tmp.group_name = itr.group_name;
        tmp.id = itr.get_pri_id();
        tmp.main_vehicle_number = itr.main_vehicle_number;
        tmp.in_white_list = itr.in_white_list;
        tmp.max_count = itr.max_count;
        _return.push_back(tmp);
    }
}