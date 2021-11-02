#include "zh_db_config.h"
#include <uuid/uuid.h>

std::string zh_rpc_util_get_timestring(time_t _time)
{
    time_t cur_time = _time;

    auto st_time = localtime(&cur_time);
    char buff[512] = "";

    sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", st_time->tm_year + 1900, st_time->tm_mon + 1, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);

    return std::string(buff);
}
std::string zh_rpc_util_get_datestring(time_t _time)
{
    auto date_time = zh_rpc_util_get_timestring(_time);
    return date_time.substr(0, 10);
}
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid)
{
    auto user_login = sqlite_orm::search_record<zh_sql_user_login>("ssid == '%s'", ssid.c_str());
    if (user_login)
    {
        auto cur_time = time(nullptr) / 3600;
        auto ret = user_login->get_parent<zh_sql_user_info>("online_user");
        if (ret && cur_time >= user_login->timestamp && (cur_time - user_login->timestamp) <= 120)
        {
            user_login->timestamp = cur_time;
            user_login->update_record();
            return ret;
        }
        else
        {
            user_login->remove_record();
        }
    }

    return std::unique_ptr<zh_sql_user_info>();
}

std::string zh_rpc_util_gen_ssid()
{
    uuid_t out;
    std::string ret;

    uuid_generate(out);
    char byte_show[3];
    for (auto itr : out)
    {
        sprintf(byte_show, "%02X", itr);
        ret.append(byte_show);
    }

    return ret;
}

std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid, long required_permission)
{
    auto ret = zh_rpc_util_get_online_user(ssid);
    if (ret)
    {
        auto permission = ret->get_parent<zh_sql_user_permission>("permission");
        if (permission && permission->key >= 0 && permission->key <= required_permission)
        {
            return ret;
        }
    }

    return std::unique_ptr<zh_sql_user_info>();
}

std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid, zh_sql_contract &_contract)
{
    auto ret = zh_rpc_util_get_online_user(ssid);
    if (ret)
    {
        auto related_contract = ret->get_parent<zh_sql_contract>("belong_contract");
        if (related_contract && related_contract->get_pri_id() == _contract.get_pri_id())
        {
            return ret;
        }
    }

    return std::unique_ptr<zh_sql_user_info>();
}
void zh_sql_vehicle_order::push_status(zh_sql_order_status &_status)
{
    if (_status.step > status)
    {
        status = _status.step;
        auto exist_status = get_children<zh_sql_order_status>("belong_order", "step == %ld", status);
        if (exist_status)
        {
            exist_status->remove_record();
        }
        _status.set_parent(*this, "belong_order");
        _status.update_record();
        update_record();
    }
    else
    {
        _status.remove_record();
    }
}