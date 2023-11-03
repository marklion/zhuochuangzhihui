#include "config_management_imp.h"


void config_management_handler::get_stuff_config(std::vector<stuff_config> &_return)
{
    auto all_record = sqlite_orm::search_record_all<sql_stuff>();
    for (auto &itr:all_record)
    {
        stuff_config tmp;
        tmp.id = itr.get_pri_id();
        tmp.stuff_name = itr.name;

        _return.push_back(tmp);
    }
}
bool config_management_handler::add_stuff_config(const stuff_config &new_one)
{
    bool ret = false;

    sql_stuff tmp;
    tmp.name = new_one.stuff_name;

    ret = tmp.insert_record();

    return ret;
}
bool config_management_handler::del_stuff_config(const int64_t id)
{
    bool ret = false;

    auto ss = sqlite_orm::search_record<sql_stuff>(id);
    if (ss)
    {
        ss->remove_record();
        ret = true;
    }

    return ret;
}
