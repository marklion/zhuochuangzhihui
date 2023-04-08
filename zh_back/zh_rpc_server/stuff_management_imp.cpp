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
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
    tmp.min_limit = stuff.min_limit;
    tmp.max_limit = stuff.max_limit;
    tmp.code = stuff.code;
    tmp.use_for_white_list = stuff.use_for_white_list;
    tmp.auto_confirm_deliver = stuff.auto_confirm_deliver;

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
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
    exist_record->max_limit = stuff.max_limit;
    exist_record->min_limit = stuff.min_limit;
    exist_record->code = stuff.code;
    exist_record->use_for_white_list = stuff.use_for_white_list;
    exist_record->auto_confirm_deliver = stuff.auto_confirm_deliver;

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
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
        auto same_contracts = sqlite_orm::search_record_all<zh_sql_contract>("name == '%s'", customer_contract->name.c_str());
        for (auto &single_con:same_contracts)
        {
            auto followed_stuff = single_con.get_all_children<zh_sql_follow_stuff>("belong_contract");
            for (auto &itr : followed_stuff)
            {
                auto stuff = itr.get_parent<zh_sql_stuff>("belong_stuff");
                if (stuff)
                {
                    query_cmd += " OR PRI_ID == " + std::to_string(stuff->get_pri_id());
                }
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
        tmp.max_limit = itr.max_limit;
        tmp.min_limit = itr.min_limit;
        tmp.code = itr.code;
        tmp.use_for_white_list = itr.use_for_white_list;
        tmp.auto_call_count = itr.auto_call_count;
        tmp.auto_confirm_deliver = itr.auto_confirm_deliver;

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
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
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
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    _return = last_active_stuff;
}

void stuff_management_handler::get_history(std::vector<number_change_point> &_return, const std::string &ssid, const std::string &stuff_name, const int64_t count)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_CASH, true);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_CASH);
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

    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_CASH, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_CASH);
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

bool stuff_management_handler::add_source_dest(const std::string &ssid, const std::string &source_dest_name, const bool is_source, const std::string &code)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto exist_record = sqlite_orm::search_record<zh_sql_stuff_source_dest>("name == '%s' AND is_source == %ld", source_dest_name.c_str(), is_source ? 1 : 0);
    if (exist_record)
    {
        exist_record->remove_record();
    }
    zh_sql_stuff_source_dest tmp;
    tmp.name = source_dest_name;
    tmp.is_source = is_source ? 1 : 0;
    tmp.id = code;
    ret = tmp.insert_record();

    return ret;
}
void stuff_management_handler::get_all_source_dest(std::vector<stuff_source_dest> &_return, const bool is_source)
{
    auto all_ssd = sqlite_orm::search_record_all<zh_sql_stuff_source_dest>("is_source == %ld", is_source ? 1 : 0);
    for (auto &itr : all_ssd)
    {
        stuff_source_dest tmp;
        tmp.id = itr.get_pri_id();
        tmp.is_source = itr.is_source;
        tmp.name = itr.name;
        tmp.code = itr.id;
        _return.push_back(tmp);
    }
}
bool stuff_management_handler::del_source_dest(const std::string &ssid, const int64_t id)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_stuff_source_dest>(id);
    if (!exist_record)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    exist_record->remove_record();

    return true;
}

void stuff_management_handler::get_white_list_stuff(std::vector<std::string> &_return)
{
    auto stuffs = sqlite_orm::search_record_all<zh_sql_stuff>("use_for_white_list != 0");
    for (auto &itr : stuffs)
    {
        _return.push_back(itr.name);
    }
}

bool stuff_management_handler::set_auto_call_count(const std::string &ssid, const std::string &stuff_name, const int64_t auto_call_count)
{
    bool ret = false;

    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }

    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", stuff_name.c_str());
    if (!stuff)
    {
        ZH_RETURN_NO_STUFF();
    }

    stuff->auto_call_count = auto_call_count;
    ret = stuff->update_record(ssid);

    return ret;
}

bool stuff_management_handler::add_stuff_inv_info(const std::string& ssid, const std::string& name)
{
    bool ret = false;

    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_stuff_inv_info>("name == '%s'", name.c_str());
    if (exist_record)
    {
        ZH_RETURN_MSG("信息已存在");
    }

    zh_sql_stuff_inv_info tmp;
    tmp.name = name;

    ret = tmp.insert_record();

    return ret;
}
void stuff_management_handler::del_stuff_inv_info(const std::string& ssid, const int64_t id)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_stuff_inv_info>(id);
    if (!exist_record)
    {
        ZH_RETURN_MSG("信息不存在");
    }
    auto all_sie = exist_record->get_all_children<zh_sql_stuff_inv_element>("belong_sii");
    for (auto &itr:all_sie)
    {
        itr.remove_record();
    }
    exist_record->remove_record();
}
bool stuff_management_handler::update_stuff_inv_info(const std::string &ssid, const stuff_inv_info &sii)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    bool has_dup = false;
    for (auto itr = sii.sie.begin(); itr != sii.sie.end(); itr++)
    {
        if (sii.sie.end() != std::find_if(
            itr + 1,
            sii.sie.end(),
            [&](const stuff_inv_element &_item){
                return _item.stuff_name == itr->stuff_name;
            }))
        {
            has_dup = true;
            break;
        }
    }
    if (has_dup)
    {
        ZH_RETURN_MSG("物料重复了");
    }

    auto exist_record = sqlite_orm::search_record<zh_sql_stuff_inv_info>(sii.id);
    if (!exist_record)
    {
        ZH_RETURN_MSG("信息不存在");
    }
    auto all_sie = exist_record->get_all_children<zh_sql_stuff_inv_element>("belong_sii");
    for (auto &itr : all_sie)
    {
        itr.remove_record();
    }

    for (auto &itr : sii.sie)
    {
        zh_sql_stuff_inv_element tmp;
        tmp.set_parent(*exist_record, "belong_sii");
        tmp.stuff_name = itr.stuff_name;
        tmp.inventory = itr.inventory;
        tmp.insert_record();
    }

    exist_record->name = sii.name;

    ret = exist_record->update_record();

    return ret;
}
void stuff_management_handler::get_stuff_inv_info(std::vector<stuff_inv_info> &_return, const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    auto all_exist_record = sqlite_orm::search_record_all<zh_sql_stuff_inv_info>();
    for (auto &itr:all_exist_record)
    {
        stuff_inv_info sii;
        sii.name = itr.name;
        sii.id = itr.get_pri_id();
        auto all_sie = itr.get_all_children<zh_sql_stuff_inv_element>("belong_sii");
        for (auto &itr_sie:all_sie)
        {
            stuff_inv_element sie;
            sie.id = itr_sie.get_pri_id();
            sie.stuff_name = itr_sie.stuff_name;
            sie.inventory = itr_sie.inventory;
            sii.sie.push_back(sie);
        }

        _return.push_back(sii);
    }
}