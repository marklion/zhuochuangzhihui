#if !defined(_ZH_DB_CONFIG_H_)
#define _ZH_DB_CONFIG_H_

#include "sqlite_orm_tree.h"
#include "CJsonObject.hpp"

std::string zh_rpc_util_get_timestring(time_t _time = time(NULL));
std::string zh_rpc_util_get_datestring(time_t _time = time(NULL));
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
        int fd_new = open((prefix + new_name).c_str(), O_WRONLY | O_CREAT, S_IREAD | S_IWRITE);
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
class zh_sql_stuff : public sql_tree_base
{
public:
    std::string name;
    double inventory;
    std::string unit;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("inventory", sqlite_orm_column::REAL, &inventory));
        ret.push_back(sqlite_orm_column("unit", sqlite_orm_column::STRING, &unit));

        return ret;
    }
    virtual std::string table_name()
    {
        return "stuff_table";
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
    int in_white_list = 0;
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

        return ret;
    }
    virtual std::string table_name()
    {
        return "vehicle_table";
    }
};

class zh_sql_order_status;
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
    long status;
    std::string stuff_name;
    double p_weight = 0;
    double m_weight = 0;
    int m_permit = 0;
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

        return ret;
    }
    virtual std::string table_name()
    {
        return "vehicle_order_table";
    }
    void push_status(zh_sql_order_status &_status);
};
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid);
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid, long required_permission);
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid, zh_sql_contract &_contract);
std::string zh_rpc_util_gen_ssid();

class zh_sql_order_status:public sql_tree_base{
public:
    std::string name;
    std::string timestamp;
    std::string user_name;
    long step = 0;
    zh_sql_order_status() {
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
    static zh_sql_order_status make_create_status(const std::string &ssid = "") {
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

#endif // _ZH_DB_CONFIG_H_
