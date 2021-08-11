#include "zh_rpc_util.h"
#include <uuid/uuid.h>

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