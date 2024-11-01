#if !defined(_ZH_DB_CONFIG_H_)
#define _ZH_DB_CONFIG_H_

#include "sqlite_orm_tree.h"
#include "../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include <functional>

#define ZH_PERMISSON_TARGET_USER "user_info"
#define ZH_PERMISSON_TARGET_ORDER "order_info"
#define ZH_PERMISSON_TARGET_CASH "cash_info"
#define ZH_PERMISSON_TARGET_FIELD "field_info"

std::string zh_rpc_util_get_timestring(time_t _time = time(NULL));
std::string zh_rpc_util_get_datestring(time_t _time = time(NULL));
time_t zh_rpc_util_get_time_by_string(const std::string &_time_string);

std::map<std::string, long> &zh_runtime_get_device_health();
class zh_sql_user_permission : public sql_tree_base
{
public:
    std::string name;
    std::string description;
    long key = -1;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name, SQLITE_ORM_COLUMN_LIMIT_UNIQ));
        ret.push_back(sqlite_orm_column("key", sqlite_orm_column::INTEGER, &key, SQLITE_ORM_COLUMN_LIMIT_UNIQ));
        ret.push_back(sqlite_orm_column("description", sqlite_orm_column::STRING, &description));

        return ret;
    }
    virtual std::string table_name()
    {
        return "user_permission_table";
    }
};

class zh_sql_file : public sql_tree_base
{
public:
    std::string name;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name, SQLITE_ORM_COLUMN_LIMIT_UNIQ));

        return ret;
    }
    virtual std::string table_name()
    {
        return "file_table";
    }
    void save_file(const std::string &tmp_name, const std::string &new_name)
    {
        std::string prefix = "/manage_dist/logo_res/";
        int fd_orig = open(tmp_name.c_str(), O_RDONLY);
        int fd_new = open((prefix + new_name).c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
        if (fd_orig >= 0 && fd_new >= 0)
        {
            long buf[100];
            ssize_t read_len = 0;
            while ((read_len = read(fd_orig, buf, sizeof(buf))) > 0)
            {
                write(fd_new, buf, read_len);
            }
        }
        if (fd_orig >= 0)
        {
            close(fd_orig);
        }
        if (fd_new >= 0)
        {
            close(fd_new);
        }
        name = new_name;
    };
};
class zh_sql_contract : public sql_tree_base
{
public:
    std::string name;
    std::string date;
    long is_sale = 0;
    std::string code;
    std::string address;
    double balance = 0;
    double credit = 0;
    zh_sql_contract()
    {
        add_parent_type<zh_sql_file>("attachment");
    }

    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("date", sqlite_orm_column::STRING, &date));
        ret.push_back(sqlite_orm_column("code", sqlite_orm_column::STRING, &code));
        ret.push_back(sqlite_orm_column("is_sale", sqlite_orm_column::INTEGER, &is_sale));
        ret.push_back(sqlite_orm_column("address", sqlite_orm_column::STRING, &address));
        ret.push_back(sqlite_orm_column("balance", sqlite_orm_column::REAL, &balance));
        ret.push_back(sqlite_orm_column("credit", sqlite_orm_column::REAL, &credit));

        return ret;
    }
    virtual std::string table_name()
    {
        return "contract_table";
    }
};

class zh_sql_user_info : public sql_tree_base
{
public:
    std::string name;
    std::string phone;
    std::string password;
    long need_change_password = 0;

    zh_sql_user_info()
    {
        add_parent_type<zh_sql_user_permission>("permission");
        add_parent_type<zh_sql_contract>("belong_contract");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("phone", sqlite_orm_column::STRING, &phone, SQLITE_ORM_COLUMN_LIMIT_UNIQ));
        ret.push_back(sqlite_orm_column("password", sqlite_orm_column::STRING, &password));
        ret.push_back(sqlite_orm_column("need_change_password", sqlite_orm_column::INTEGER, &need_change_password));

        return ret;
    }
    virtual std::string table_name()
    {
        return "user_info_table";
    }
};

class zh_sql_user_login : public sql_tree_base
{
public:
    std::string ssid;
    long timestamp = 0;
    zh_sql_user_login()
    {
        add_parent_type<zh_sql_user_info>("online_user");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("ssid", sqlite_orm_column::STRING, &ssid));
        ret.push_back(sqlite_orm_column("timestamp", sqlite_orm_column::INTEGER, &timestamp));

        return ret;
    }
    virtual std::string table_name()
    {
        return "user_login_table";
    }
};

class zh_sql_stuff_inv_info:public sql_tree_base
{
public:
    std::string name;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));

        return ret;
    }
    virtual std::string table_name()
    {
        return "stuff_inv_info_table";
    }
};

class zh_sql_stuff_inv_element:public sql_tree_base
{
public:
    std::string stuff_name;
    double inventory = 0;
    zh_sql_stuff_inv_element() {
        add_parent_type<zh_sql_stuff_inv_info>("belong_sii");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("stuff_name", sqlite_orm_column::STRING, &stuff_name));
        ret.push_back(sqlite_orm_column("inventory", sqlite_orm_column::REAL, &inventory));

        return ret;
    }
    virtual std::string table_name()
    {
        return "stuff_inv_element_table";
    }
};
class zh_sql_stuff : public sql_tree_base
{
public:
    std::string name;
    double inventory = 0;
    std::string unit;
    long need_enter_weight = 0;
    double price = 0;
    double expect_weight = 0;
    long need_manual_scale = 0;
    double min_limit = 45;
    double max_limit = 49.5;
    std::string code;
    long use_for_white_list = 0;
    long auto_call_count = 0;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("inventory", sqlite_orm_column::REAL, &inventory));
        ret.push_back(sqlite_orm_column("unit", sqlite_orm_column::STRING, &unit));
        ret.push_back(sqlite_orm_column("need_enter_weight", sqlite_orm_column::INTEGER, &need_enter_weight));
        ret.push_back(sqlite_orm_column("price", sqlite_orm_column::REAL, &price));
        ret.push_back(sqlite_orm_column("expect_weight", sqlite_orm_column::REAL, &expect_weight));
        ret.push_back(sqlite_orm_column("need_manual_scale", sqlite_orm_column::INTEGER, &need_manual_scale));
        ret.push_back(sqlite_orm_column("min_limit", sqlite_orm_column::REAL, &min_limit));
        ret.push_back(sqlite_orm_column("max_limit", sqlite_orm_column::REAL, &max_limit));
        ret.push_back(sqlite_orm_column("code", sqlite_orm_column::STRING, &code));
        ret.push_back(sqlite_orm_column("use_for_white_list", sqlite_orm_column::INTEGER, &use_for_white_list));
        ret.push_back(sqlite_orm_column("auto_call_count", sqlite_orm_column::INTEGER, &auto_call_count));

        return ret;
    }
    virtual std::string table_name()
    {
        return "stuff_table";
    }
    bool insert_record(const std::string &ssid);
    bool update_record(const std::string &ssid);
    void remove_record(const std::string &ssid);
};

class zh_sql_follow_stuff : public sql_tree_base
{
public:
    zh_sql_follow_stuff()
    {
        add_parent_type<zh_sql_contract>("belong_contract");
        add_parent_type<zh_sql_stuff>("belong_stuff");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        return ret;
    }
    virtual std::string table_name()
    {
        return "follow_stuff_table";
    }
};
class zh_sql_vehicle : public sql_tree_base
{
public:
    std::string main_vehicle_number;
    std::string behind_vehicle_number;
    std::string driver_name;
    std::string driver_id;
    std::string driver_phone;
    std::string company_name;
    std::string group_name;
    long in_white_list = 0;
    double max_count = 35;
    std::string use_stuff;
    std::string use_date;
    long in_black_list = 0;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("main_vehicle_number", sqlite_orm_column::STRING, &main_vehicle_number));
        ret.push_back(sqlite_orm_column("behind_vehicle_number", sqlite_orm_column::STRING, &behind_vehicle_number));
        ret.push_back(sqlite_orm_column("driver_name", sqlite_orm_column::STRING, &driver_name));
        ret.push_back(sqlite_orm_column("driver_id", sqlite_orm_column::STRING, &driver_id));
        ret.push_back(sqlite_orm_column("driver_phone", sqlite_orm_column::STRING, &driver_phone));
        ret.push_back(sqlite_orm_column("company_name", sqlite_orm_column::STRING, &company_name));
        ret.push_back(sqlite_orm_column("group_name", sqlite_orm_column::STRING, &group_name));
        ret.push_back(sqlite_orm_column("in_white_list", sqlite_orm_column::INTEGER, &in_white_list));
        ret.push_back(sqlite_orm_column("max_count", sqlite_orm_column::REAL, &max_count));
        ret.push_back(sqlite_orm_column("use_stuff", sqlite_orm_column::STRING, &use_stuff));
        ret.push_back(sqlite_orm_column("use_date", sqlite_orm_column::STRING, &use_date));
        ret.push_back(sqlite_orm_column("in_black_list", sqlite_orm_column::INTEGER, &in_black_list));

        return ret;
    }
    virtual std::string table_name()
    {
        return "vehicle_table";
    }
};

class zh_sql_order_status;
class zh_sql_vehicle_order;
struct zh_order_save_hook{
    std::function<bool (zh_sql_vehicle_order &)> before_hook = [](zh_sql_vehicle_order &)->bool{return true;};
    std::function<void (zh_sql_vehicle_order &)> after_hook = [](zh_sql_vehicle_order &){};
    zh_order_save_hook(const std::function<bool (zh_sql_vehicle_order &)> &_before_hook,
    const std::function<void (zh_sql_vehicle_order &)> &_after_hook):before_hook(_before_hook),after_hook(_after_hook){}
    zh_order_save_hook():before_hook([](zh_sql_vehicle_order &)->bool{return true;}),after_hook([](zh_sql_vehicle_order &){}) {}
};
class zh_sql_vehicle_order : public sql_tree_base
{
public:
    std::string order_number;
    std::string main_vehicle_number;
    std::string behind_vehicle_number;
    std::string driver_name;
    std::string driver_id;
    std::string driver_phone;
    std::string company_name;
    long status = -1;
    std::string stuff_name;
    double p_weight = 0;
    double m_weight = 0;
    long m_permit = 0;
    long m_registered = 0;
    long m_called = 0;
    double enter_weight = 0;
    long need_enter_weight = 0;
    std::string enter_nvr_ip;
    std::string exit_nvr_ip;
    std::string p_nvr_ip1;
    std::string m_nvr_ip1;
    std::string p_nvr_ip2;
    std::string m_nvr_ip2;
    std::string enter_cam_time;
    std::string exit_cam_time;
    std::string p_cam_time;
    std::string m_cam_time;
    std::string company_address;
    std::string use_for;
    double max_count = 35;
    long check_in_timestamp = 0;
    std::string end_time;
    std::string source_dest_name;
    long call_timestamp = 0;
    std::string bl_number;
    std::string extra_info;
    std::string err_string;
    double price = 0;
    std::string call_user_name;
    long from_api = 0;
    std::string seal_no;
    std::string max_load;
    long is_scaling = 0;
    std::string bound_inv_name;
    std::string p_scale_name;
    std::string m_scale_name;
    std::string p_gate_name;
    std::string m_gate_name;
    zh_sql_vehicle_order()
    {
        add_parent_type<zh_sql_file>("attachment");
        add_parent_type<zh_sql_file>("enter_weight_attachment");
        add_parent_type<zh_sql_file>("enter_picture");
        add_parent_type<zh_sql_file>("exit_picture");
        add_parent_type<zh_sql_file>("p_picture");
        add_parent_type<zh_sql_file>("m_picture");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("main_vehicle_number", sqlite_orm_column::STRING, &main_vehicle_number));
        ret.push_back(sqlite_orm_column("behind_vehicle_number", sqlite_orm_column::STRING, &behind_vehicle_number));
        ret.push_back(sqlite_orm_column("driver_name", sqlite_orm_column::STRING, &driver_name));
        ret.push_back(sqlite_orm_column("driver_id", sqlite_orm_column::STRING, &driver_id));
        ret.push_back(sqlite_orm_column("driver_phone", sqlite_orm_column::STRING, &driver_phone));
        ret.push_back(sqlite_orm_column("company_name", sqlite_orm_column::STRING, &company_name));
        ret.push_back(sqlite_orm_column("order_number", sqlite_orm_column::STRING, &order_number));
        ret.push_back(sqlite_orm_column("status", sqlite_orm_column::INTEGER, &status));
        ret.push_back(sqlite_orm_column("stuff_name", sqlite_orm_column::STRING, &stuff_name));
        ret.push_back(sqlite_orm_column("p_weight", sqlite_orm_column::REAL, &p_weight));
        ret.push_back(sqlite_orm_column("m_weight", sqlite_orm_column::REAL, &m_weight));
        ret.push_back(sqlite_orm_column("m_permit", sqlite_orm_column::INTEGER, &m_permit));
        ret.push_back(sqlite_orm_column("m_registered", sqlite_orm_column::INTEGER, &m_registered));
        ret.push_back(sqlite_orm_column("m_called", sqlite_orm_column::INTEGER, &m_called));
        ret.push_back(sqlite_orm_column("enter_weight", sqlite_orm_column::REAL, &enter_weight));
        ret.push_back(sqlite_orm_column("need_enter_weight", sqlite_orm_column::INTEGER, &need_enter_weight));
        ret.push_back(sqlite_orm_column("enter_nvr_ip", sqlite_orm_column::STRING, &enter_nvr_ip));
        ret.push_back(sqlite_orm_column("exit_nvr_ip", sqlite_orm_column::STRING, &exit_nvr_ip));
        ret.push_back(sqlite_orm_column("p_nvr_ip1", sqlite_orm_column::STRING, &p_nvr_ip1));
        ret.push_back(sqlite_orm_column("m_nvr_ip1", sqlite_orm_column::STRING, &m_nvr_ip1));
        ret.push_back(sqlite_orm_column("p_nvr_ip2", sqlite_orm_column::STRING, &p_nvr_ip2));
        ret.push_back(sqlite_orm_column("m_nvr_ip2", sqlite_orm_column::STRING, &m_nvr_ip2));
        ret.push_back(sqlite_orm_column("enter_cam_time", sqlite_orm_column::STRING, &enter_cam_time));
        ret.push_back(sqlite_orm_column("exit_cam_time", sqlite_orm_column::STRING, &exit_cam_time));
        ret.push_back(sqlite_orm_column("p_cam_time", sqlite_orm_column::STRING, &p_cam_time));
        ret.push_back(sqlite_orm_column("m_cam_time", sqlite_orm_column::STRING, &m_cam_time));
        ret.push_back(sqlite_orm_column("company_address", sqlite_orm_column::STRING, &company_address));
        ret.push_back(sqlite_orm_column("use_for", sqlite_orm_column::STRING, &use_for));
        ret.push_back(sqlite_orm_column("max_count", sqlite_orm_column::REAL, &max_count));
        ret.push_back(sqlite_orm_column("check_in_timestamp", sqlite_orm_column::INTEGER, &check_in_timestamp));
        ret.push_back(sqlite_orm_column("end_time", sqlite_orm_column::STRING, &end_time));
        ret.push_back(sqlite_orm_column("source_dest_name", sqlite_orm_column::STRING, &source_dest_name));
        ret.push_back(sqlite_orm_column("call_timestamp", sqlite_orm_column::INTEGER, &call_timestamp));
        ret.push_back(sqlite_orm_column("bl_number", sqlite_orm_column::STRING, &bl_number));
        ret.push_back(sqlite_orm_column("extra_info", sqlite_orm_column::STRING, &extra_info));
        ret.push_back(sqlite_orm_column("err_string", sqlite_orm_column::STRING, &err_string));
        ret.push_back(sqlite_orm_column("price", sqlite_orm_column::REAL, &price));
        ret.push_back(sqlite_orm_column("call_user_name", sqlite_orm_column::STRING, &call_user_name));
        ret.push_back(sqlite_orm_column("from_api", sqlite_orm_column::INTEGER, &from_api));
        ret.push_back(sqlite_orm_column("seal_no", sqlite_orm_column::STRING, &seal_no));
        ret.push_back(sqlite_orm_column("max_load", sqlite_orm_column::STRING, &max_load));
        ret.push_back(sqlite_orm_column("is_scaling", sqlite_orm_column::INTEGER, &is_scaling));
        ret.push_back(sqlite_orm_column("bound_inv_name", sqlite_orm_column::STRING, &bound_inv_name));
        ret.push_back(sqlite_orm_column("p_scale_name", sqlite_orm_column::STRING, &p_scale_name));
        ret.push_back(sqlite_orm_column("m_scale_name", sqlite_orm_column::STRING, &m_scale_name));
        ret.push_back(sqlite_orm_column("p_gate_name", sqlite_orm_column::STRING, &p_gate_name));
        ret.push_back(sqlite_orm_column("m_gate_name", sqlite_orm_column::STRING, &m_gate_name));

        return ret;
    }
    virtual std::string table_name()
    {
        return "vehicle_order_table";
    }
    void push_status(
        zh_sql_order_status &_status, const zh_order_save_hook &_save_hook = zh_order_save_hook());
};
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid);
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid, long required_permission);
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid, zh_sql_contract &_contract);
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid, const std::string &_permission_target, bool _is_read = false);
std::string zh_rpc_util_gen_ssid();

class zh_sql_history_data : public sql_tree_base
{
public:
    double change_value = 0;
    double new_value = 0;
    std::string timestamp;
    std::string reason;

    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("change_value", sqlite_orm_column::REAL, &change_value));
        ret.push_back(sqlite_orm_column("new_value", sqlite_orm_column::REAL, &new_value));
        ret.push_back(sqlite_orm_column("timestamp", sqlite_orm_column::STRING, &timestamp));
        ret.push_back(sqlite_orm_column("reason", sqlite_orm_column::STRING, &reason));
        return ret;
    }
};
class zh_sql_balance_point : public zh_sql_history_data
{
public:
    zh_sql_balance_point()
    {
        add_parent_type<zh_sql_contract>("belong_contract");
    }
    virtual std::string table_name()
    {
        return "balance_point_table";
    }
};

class zh_sql_price_point : public zh_sql_history_data
{
public:
    zh_sql_price_point()
    {
        add_parent_type<zh_sql_stuff>("belong_stuff");
    }
    virtual std::string table_name()
    {
        return "price_point_table";
    }
};
class zh_sql_order_status : public sql_tree_base
{
public:
    std::string name;
    std::string timestamp;
    std::string user_name;
    long step = 0;
    zh_sql_order_status()
    {
        add_parent_type<zh_sql_vehicle_order>("belong_order");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("timestamp", sqlite_orm_column::STRING, &timestamp));
        ret.push_back(sqlite_orm_column("user_name", sqlite_orm_column::STRING, &user_name));
        ret.push_back(sqlite_orm_column("step", sqlite_orm_column::INTEGER, &step));
        return ret;
    }
    virtual std::string table_name()
    {
        return "order_status_table";
    }
    static zh_sql_order_status make_create_status(const std::string &ssid = "")
    {
        zh_sql_order_status ret;
        ret.name = "创建";
        ret.timestamp = zh_rpc_util_get_timestring();
        if (ssid.length() > 0)
        {
            auto opt_user = zh_rpc_util_get_online_user(ssid);
            if (opt_user)
            {
                ret.user_name = opt_user->name;
            }
        }
        else
        {
            ret.user_name = "(自动)";
        }
        ret.step = 0;
        ret.insert_record();

        return ret;
    }
    static zh_sql_order_status make_before_come_status(const std::string &ssid = "")
    {
        zh_sql_order_status ret;
        ret.name = "等待入场";
        ret.timestamp = zh_rpc_util_get_timestring();
        if (ssid.length() > 0)
        {
            auto opt_user = zh_rpc_util_get_online_user(ssid);
            if (opt_user)
            {
                ret.user_name = opt_user->name;
            }
        }
        else
        {
            ret.user_name = "(自动)";
        }
        ret.step = 1;
        ret.insert_record();

        return ret;
    }
    static zh_sql_order_status make_in_status(const std::string &ssid = "")
    {
        zh_sql_order_status ret;
        ret.name = "进场";
        ret.timestamp = zh_rpc_util_get_timestring();
        if (ssid.length() > 0)
        {
            auto opt_user = zh_rpc_util_get_online_user(ssid);
            if (opt_user)
            {
                ret.user_name = opt_user->name;
            }
        }
        else
        {
            ret.user_name = "(自动)";
        }
        ret.step = 2;
        ret.insert_record();

        return ret;
    }
    static zh_sql_order_status make_p_status(const std::string &ssid = "")
    {
        zh_sql_order_status ret;
        ret.name = "一次称重";
        ret.timestamp = zh_rpc_util_get_timestring();
        if (ssid.length() > 0)
        {
            auto opt_user = zh_rpc_util_get_online_user(ssid);
            if (opt_user)
            {
                ret.user_name = opt_user->name;
            }
        }
        else
        {
            ret.user_name = "(自动)";
        }
        ret.step = 3;
        ret.insert_record();

        return ret;
    }
    static zh_sql_order_status make_m_status(const std::string &ssid = "")
    {
        zh_sql_order_status ret;
        ret.name = "二次称重";
        ret.timestamp = zh_rpc_util_get_timestring();
        if (ssid.length() > 0)
        {
            auto opt_user = zh_rpc_util_get_online_user(ssid);
            if (opt_user)
            {
                ret.user_name = opt_user->name;
            }
        }
        else
        {
            ret.user_name = "(自动)";
        }
        ret.step = 4;
        ret.insert_record();

        return ret;
    }
    static zh_sql_order_status make_out_status(const std::string &ssid = "")
    {
        zh_sql_order_status ret;
        ret.name = "出场";
        ret.timestamp = zh_rpc_util_get_timestring();
        if (ssid.length() > 0)
        {
            auto opt_user = zh_rpc_util_get_online_user(ssid);
            if (opt_user)
            {
                ret.user_name = opt_user->name;
            }
        }
        else
        {
            ret.user_name = "(自动)";
        }
        ret.step = 5;
        ret.insert_record();

        return ret;
    }
    static zh_sql_order_status make_end_status(const std::string &ssid = "")
    {
        zh_sql_order_status ret;
        ret.name = "结束";
        ret.timestamp = zh_rpc_util_get_timestring();
        if (ssid.length() > 0)
        {
            auto opt_user = zh_rpc_util_get_online_user(ssid);
            if (opt_user)
            {
                ret.user_name = opt_user->name;
            }
        }
        else
        {
            ret.user_name = "(自动)";
        }
        ret.step = 100;
        ret.insert_record();

        return ret;
    }
};

class zh_sql_driver_self_order : public sql_tree_base
{
public:
    std::string main_vehicle_number;
    std::string driver_id;
    std::string driver_name;
    std::string driver_phone;
    std::string stuff_name;
    zh_sql_driver_self_order()
    {
        add_parent_type<zh_sql_user_info>("belong_user");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("main_vehicle_number", sqlite_orm_column::STRING, &main_vehicle_number));
        ret.push_back(sqlite_orm_column("driver_id", sqlite_orm_column::STRING, &driver_id));
        ret.push_back(sqlite_orm_column("driver_name", sqlite_orm_column::STRING, &driver_name));
        ret.push_back(sqlite_orm_column("driver_phone", sqlite_orm_column::STRING, &driver_phone));
        ret.push_back(sqlite_orm_column("stuff_name", sqlite_orm_column::STRING, &stuff_name));

        return ret;
    }
    virtual std::string table_name()
    {
        return "driver_self_order_table";
    }
};

class zh_sql_prompt_image: public sql_tree_base{
public:
    zh_sql_prompt_image()
    {
        add_parent_type<zh_sql_file>("attachment");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;

        return ret;
    }
    virtual std::string table_name()
    {
        return "prompt_image_table";
    }
};

std::string zh_double2string_reserve2(double _value);

class zh_sql_stuff_source_dest : public sql_tree_base
{
public:
    std::string name;
    long is_source = 0;
    std::string id;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("is_source", sqlite_orm_column::INTEGER, &is_source));
        ret.push_back(sqlite_orm_column("id", sqlite_orm_column::STRING, &id));
        return ret;
    }

    virtual std::string table_name()
    {
        return "stuff_source_dest_table";
    }
};

class zh_sql_white_record:public sql_tree_base {
public:
    std::string vehicle_number;
    std::string date;
    double weight = 0;
    std::string use_stuff;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("vehicle_number", sqlite_orm_column::STRING, &vehicle_number));
        ret.push_back(sqlite_orm_column("date", sqlite_orm_column::STRING, &date));
        ret.push_back(sqlite_orm_column("weight", sqlite_orm_column::REAL, &weight));
        ret.push_back(sqlite_orm_column("use_stuff", sqlite_orm_column::STRING, &use_stuff));
        return ret;
    }

    virtual std::string table_name()
    {
        return "white_record_table";
    }
};

class zh_sql_contract_stuff_price :public sql_tree_base{
public:
    std::string customer_name;
    std::string stuff_name;
    double price = 0;

    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("customer_name", sqlite_orm_column::STRING, &customer_name));
        ret.push_back(sqlite_orm_column("stuff_name", sqlite_orm_column::STRING, &stuff_name));
        ret.push_back(sqlite_orm_column("price", sqlite_orm_column::REAL, &price));
        return ret;
    }

    virtual std::string table_name()
    {
        return "contract_stuff_price_table";
    }
};

class zh_sql_single_stuff_price_point:public zh_sql_history_data {
public:
    zh_sql_single_stuff_price_point() {
        add_parent_type<zh_sql_contract_stuff_price>("belong_price");
    }

    virtual std::string table_name()
    {
        return "single_stuff_price_point_table";
    }
};

class zh_sql_permission_target:public sql_tree_base{
public:
    std::string target;
    long is_read = 0;
    zh_sql_permission_target() {
        add_parent_type<zh_sql_user_info>("belong_user");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("target", sqlite_orm_column::STRING, &target));
        ret.push_back(sqlite_orm_column("is_read", sqlite_orm_column::INTEGER, &is_read));
        return ret;
    }

    virtual std::string table_name()
    {
        return "permisson_target_table";
    }
};

class zh_sql_video_path:public sql_tree_base{
public:
    std::string name;
    std::string path;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("path", sqlite_orm_column::STRING, &path));
        return ret;
    }

    virtual std::string table_name()
    {
        return "video_path_table";
    }
};

#endif // _ZH_DB_CONFIG_H_
