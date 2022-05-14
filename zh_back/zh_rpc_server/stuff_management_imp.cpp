#include "stuff_management_imp.h"
#include "zh_rpc_util.h"
#include <fstream>

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
    tmp.need_enter_weight = stuff.need_enter_weight;
    tmp.expect_weight = stuff.expect_weight;
    tmp.need_manual_scale = stuff.need_manual_scale;

    ret = tmp.insert_record(ssid);
    if (ret)
    {
        last_active_stuff = tmp.name;
    }

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
    exist_record->need_enter_weight = stuff.need_enter_weight;
    exist_record->expect_weight = stuff.expect_weight;
    exist_record->need_manual_scale = stuff.need_manual_scale;

    ret = exist_record->update_record(ssid);
    if (ret)
    {
        last_active_stuff = exist_record->name;
    }

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
    auto followed_stuff = exist_record->get_all_children<zh_sql_follow_stuff>("belong_stuff");
    for (auto &itr:followed_stuff)
    {
        itr.remove_record();
    }

    return ret;
}
void stuff_management_handler::get_all_stuff(std::vector<stuff_info> &_return, const std::string &ssid, const std::string& user_name)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid);
    if (!opt_user && user_name.length() == 0)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    std::string query_cmd;
    std::unique_ptr<zh_sql_contract> customer_contract;
    if (opt_user)
    {
        customer_contract.reset(opt_user->get_parent<zh_sql_contract>("belong_contract").release());
    }
    else
    {
        customer_contract.reset(sqlite_orm::search_record<zh_sql_contract>("name == '%s'", user_name.c_str()).release());
    }
    if (customer_contract)
    {
        query_cmd = "(PRI_ID == 0";
        auto followed_stuff = customer_contract->get_all_children<zh_sql_follow_stuff>("belong_contract");
        for (auto &itr : followed_stuff)
        {
            auto stuff = itr.get_parent<zh_sql_stuff>("belong_stuff");
            if (stuff)
            {
                query_cmd += " OR PRI_ID == " + std::to_string(stuff->get_pri_id());
            }
        }
        query_cmd += ")";
    }
    auto all_stuff = sqlite_orm::search_record_all<zh_sql_stuff>(query_cmd);
    for (auto &itr : all_stuff)
    {
        stuff_info tmp;
        tmp.id = itr.get_pri_id();
        tmp.inventory = itr.inventory;
        tmp.name = itr.name;
        tmp.unit = itr.unit;
        tmp.need_enter_weight = itr.need_enter_weight;
        tmp.price = itr.price;
        tmp.expect_weight = itr.expect_weight;
        tmp.need_manual_scale = itr.need_manual_scale;

        _return.push_back(tmp);
    }
}
static std::vector<std::string> split_string(const std::string &str, const std::string &pattern)
{
    using namespace std;
    vector<string> res;
    if (str == "")
        return res;
    //在字符串末尾也加入分隔符，方便截取最后一段
    string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while (pos != strs.npos)
    {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        //去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos + pattern.size(), strs.size());
        pos = strs.find(pattern);
    }

    return res;
}
time_t convert_time_from_string(const std::string &_time)
{
    tm tm_;
    strptime(_time.c_str(), "%Y/%m/%d %H:%M:%S", &tm_);
    tm_.tm_isdst = -1;
    return mktime(&tm_);
}
std::vector<stuff_change_point> collect_change_point(const std::string &_name, const std::string &_begin, const std::string &_end)
{
    std::vector<stuff_change_point> ret;
    tdf_log tmp_log("stuff history");

    std::ifstream audit_log_file("/manage_dist/logo_res/audit_log.log", std::ios::in);
    std::string single_rec;
    while (std::getline(audit_log_file, single_rec))
    {
        tmp_log.log("single_rec:%s", single_rec.c_str());
        stuff_change_point tmp;
        tmp.date = split_string(single_rec, "[操作审计]")[0];
        if (convert_time_from_string(tmp.date) > convert_time_from_string(_begin) && convert_time_from_string(tmp.date) < convert_time_from_string(_end))
        {
            std::string stuff_name;
            if (single_rec.find('|') != std::string::npos)
            {
                auto meta_str = split_string(single_rec, "|");
                tmp_log.log("meta_str[2]:%s", meta_str[2].c_str());
                stuff_name = split_string(meta_str[2], "：")[2];
                tmp.inventory = std::stod(*(split_string(single_rec, "库存：").end() - 1));
            }
            else if (single_rec.find("新增物料") != std::string::npos)
            {
                auto insert_meta = split_string(single_rec, "新增物料：");
                for (auto &itr : insert_meta)
                {
                    tmp_log.log("新增--》%s", itr.c_str());
                }
                auto stuff_meta_info = split_string(insert_meta[1], "，");
                for (auto &itr : stuff_meta_info)
                {
                    tmp_log.log("物料--》%s", itr.c_str());
                }
                stuff_name = split_string(insert_meta[1], "，")[0];
                tmp.inventory = std::stod(*(split_string(single_rec, "库存为：").end() - 1));
            }
            tmp_log.log("name:%s,invetory:%f", stuff_name.c_str(), tmp.inventory);
            if (_name == stuff_name)
            {
                ret.push_back(tmp);
            }
        }
    }

    return ret;
}

void stuff_management_handler::get_change_points_for_range(std::vector<stuff_history> &_return, const std::string &ssid, const std::string &start_date, const std::string &end_date)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 2);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto all_stuff = sqlite_orm::search_record_all<zh_sql_stuff>();
    for (auto &itr : all_stuff)
    {
        stuff_history tmp;
        tmp.stuff.id = itr.get_pri_id();
        tmp.stuff.inventory = itr.inventory;
        tmp.stuff.name = itr.name;
        tmp.stuff.unit = itr.unit;
        tmp.change_point = collect_change_point(itr.name, start_date, end_date);
        _return.push_back(tmp);
    }
}

void stuff_management_handler::get_last_active(std::string &_return, const std::string &ssid)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, 2);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    _return = last_active_stuff;
}

void stuff_management_handler::get_history(std::vector<number_change_point> &_return, const std::string &ssid, const std::string &stuff_name, const int64_t count)
{
    auto user = zh_rpc_util_get_online_user(ssid, 2);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", stuff_name.c_str());
    if (!stuff)
    {
        ZH_RETURN_NO_STUFF();
    }
    auto points = stuff->get_all_children<zh_sql_price_point>("belong_stuff", "PRI_ID != 0 ORDER BY PRI_ID DESC LIMIT 10 OFFSET %ld", count);
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
bool stuff_management_handler::change_price(const std::string &ssid, const std::string &stuff_name, const double new_value)
{
    bool ret = false;

    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", stuff_name.c_str());
    if (!stuff)
    {
        ZH_RETURN_NO_STUFF();
    }

    zh_sql_price_point tmp;
    tmp.change_value = new_value - stuff->price;
    tmp.new_value = new_value;
    tmp.timestamp = zh_rpc_util_get_timestring();
    tmp.set_parent(*stuff, "belong_stuff");

    if (tmp.insert_record())
    {
        stuff->price = new_value;
        ret = stuff->update_record(ssid);
    }

    return ret;
}

void stuff_management_handler::get_stuff(stuff_info &_return, const std::string &ssid, const std::string &stuff_name)
{
    auto user = zh_rpc_util_get_online_user(ssid);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", stuff_name.c_str());
    if (!stuff)
    {
        ZH_RETURN_NO_STUFF();
    }
    _return.expect_weight = stuff->expect_weight;
    _return.id = stuff->get_pri_id();
    _return.inventory = stuff->inventory;
    _return.name = stuff->name;
    _return.need_enter_weight = stuff->need_enter_weight;
    _return.price = stuff->price;
    _return.unit = stuff->unit;
    _return.need_manual_scale = stuff->need_manual_scale;
}

bool stuff_management_handler::add_source_dest(const std::string &ssid, const std::string &source_dest_name, const bool is_source)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto exist_record = sqlite_orm::search_record<zh_sql_stuff_source_dest>("name == '%s' AND is_source == %ld", source_dest_name.c_str(), is_source?1:0);
    if (exist_record)
    {
        ZH_RETURN_MSG("记录已存在");
    }
    zh_sql_stuff_source_dest tmp;
    tmp.name = source_dest_name;
    tmp.is_source = is_source?1:0;
    ret = tmp.insert_record();

    return ret;
}
void stuff_management_handler::get_all_source_dest(std::vector<stuff_source_dest> &_return, const bool is_source)
{
    auto all_ssd = sqlite_orm::search_record_all<zh_sql_stuff_source_dest>("is_source == %ld", is_source?1:0);
    for (auto &itr:all_ssd)
    {
        stuff_source_dest tmp;
        tmp.id = itr.get_pri_id();
        tmp.is_source = itr.is_source;
        tmp.name = itr.name;
        _return.push_back(tmp);
    }
}
bool stuff_management_handler::del_source_dest(const std::string &ssid, const int64_t id)
{
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_stuff_source_dest>(id);
    if (!exist_record)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    exist_record->remove_record();

    return true;
}