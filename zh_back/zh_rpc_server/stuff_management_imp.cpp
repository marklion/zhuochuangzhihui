#include "stuff_management_imp.h"
#include "zh_rpc_util.h"

stuff_management_handler *stuff_management_handler::m_inst = nullptr;

bool stuff_is_dup(const stuff_info &stuff)
{
    bool ret = false;

    auto exist_record = sqlite_orm::search_record<zh_sql_stuff>("name == '%s' AND PRI_ID != %ld", stuff.name.c_str(), stuff.id);
    if (exist_record)
    {
        ret = true;
    }

    return ret;
}

bool stuff_management_handler::add_stuff(const std::string &ssid, const stuff_info &stuff)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    if (stuff_is_dup(stuff))
    {
        ZH_RETURN_DUP_STUFF();
    }

    zh_sql_stuff tmp;
    tmp.inventory = stuff.inventory;
    tmp.name = stuff.name;
    tmp.unit = stuff.unit;

    ret = tmp.insert_record(ssid);

    return ret;
}
bool stuff_management_handler::update_stuff(const std::string &ssid, const stuff_info &stuff)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_stuff>(stuff.id);
    if (!exist_record)
    {
        ZH_RETURN_NO_STUFF();
    }
    if (stuff_is_dup(stuff))
    {
        ZH_RETURN_DUP_STUFF();
    }
    exist_record->inventory = stuff.inventory;
    exist_record->name = stuff.name;
    exist_record->unit = stuff.unit;

    ret = exist_record->update_record(ssid);

    return ret;
}
bool stuff_management_handler::del_stuff(const std::string &ssid, const int64_t id)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_stuff>(id);
    if (!exist_record)
    {
        ZH_RETURN_NO_STUFF();
    }

    exist_record->remove_record(ssid);
    ret = true;

    return ret;
}
void stuff_management_handler::get_all_stuff(std::vector<stuff_info> &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto all_stuff = sqlite_orm::search_record_all<zh_sql_stuff>();
    for (auto &itr:all_stuff)
    {
        stuff_info tmp;
        tmp.id = itr.get_pri_id();
        tmp.inventory = itr.inventory;
        tmp.name = itr.name;
        tmp.unit = itr.unit;

        _return.push_back(tmp);
    }
}