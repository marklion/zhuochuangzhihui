#if !defined(_ZH_DB_CONFIG_H_)
#define _ZH_DB_CONFIG_H_

#include "sqlite_orm_tree.h"
#include "../utils/CJsonObject.hpp"
#include "../utils/Base64.h"
#include "../utils/clipp.h"
#include <functional>

class sql_stuff : public sql_tree_base
{
public:
    std::string name;
    double inventory = 0;
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
};

class sql_user : public sql_tree_base
{
public:
    std::string name;
    std::string phone;
    std::string md5_password;
    long is_external = 0;
    std::string online_token;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("phone", sqlite_orm_column::STRING, &phone));
        ret.push_back(sqlite_orm_column("md5_password", sqlite_orm_column::STRING, &md5_password));
        ret.push_back(sqlite_orm_column("online_token", sqlite_orm_column::STRING, &online_token));
        ret.push_back(sqlite_orm_column("is_external", sqlite_orm_column::INTEGER, &is_external));

        return ret;
    }
    virtual std::string table_name()
    {
        return "user_table";
    }
};

class sql_permission : public sql_tree_base
{
public:
    std::string permission_name;
    std::string text_name;
    long is_module = 0;
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;
        ret.push_back(sqlite_orm_column("permission_name", sqlite_orm_column::STRING, &permission_name));
        ret.push_back(sqlite_orm_column("text_name", sqlite_orm_column::STRING, &text_name));
        ret.push_back(sqlite_orm_column("is_module", sqlite_orm_column::INTEGER, &is_module));

        return ret;
    }
    virtual std::string table_name()
    {
        return "permission_table";
    }
};

class sql_role : public sql_tree_base
{
public:
    std::string name;
    std::string has_user_id;
    std::string has_permission_id;
    long read_only = 0;

    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;

        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("has_user_id", sqlite_orm_column::STRING, &has_user_id));
        ret.push_back(sqlite_orm_column("has_permission_id", sqlite_orm_column::STRING, &has_permission_id));
        ret.push_back(sqlite_orm_column("read_only", sqlite_orm_column::INTEGER, &read_only));

        return ret;
    }
    virtual std::string table_name()
    {
        return "role_table";
    }
};

class sql_device_driver : public sql_tree_base
{
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
        return "device_driver_table";
    }
};
class sql_device_meta;
class sql_device_set : public sql_tree_base
{
public:
    std::string name;
    long is_scale = 0;
    sql_device_set(){
        add_parent_type<sql_device_meta>("front_plate_cam");
        add_parent_type<sql_device_meta>("back_plate_cam");
        add_parent_type<sql_device_meta>("front_video_cam");
        add_parent_type<sql_device_meta>("back_video_cam");
        add_parent_type<sql_device_meta>("front_led");
        add_parent_type<sql_device_meta>("back_led");
        add_parent_type<sql_device_meta>("front_speaker");
        add_parent_type<sql_device_meta>("back_speaker");
        add_parent_type<sql_device_meta>("front_gate");
        add_parent_type<sql_device_meta>("back_gate");
        add_parent_type<sql_device_meta>("front_id_reader");
        add_parent_type<sql_device_meta>("back_id_reader");
        add_parent_type<sql_device_meta>("front_qr_reader");
        add_parent_type<sql_device_meta>("back_qr_reader");
        add_parent_type<sql_device_meta>("front_printer");
        add_parent_type<sql_device_meta>("back_printer");
        add_parent_type<sql_device_meta>("scale");
    }
    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;

        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("is_scale", sqlite_orm_column::INTEGER, &is_scale));

        return ret;
    }
    bool is_empty_set();
    virtual std::string table_name()
    {
        return "device_set_table";
    }
};
class sql_device_meta : public sql_tree_base
{
public:
    std::string name;
    std::string args;

    sql_device_meta()
    {
        add_parent_type<sql_device_driver>("driver");
    }

    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;

        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("args", sqlite_orm_column::STRING, &args));

        return ret;
    }
    virtual std::string table_name()
    {
        return "device_meta_table";
    }
};

class sql_contract : public sql_tree_base
{
public:
    std::string name;
    long is_sale = 0;
    std::string attachment;
    std::string code;
    std::string admin_name_phone;
    double balance = 0;
    double credit = 0;
    std::string follow_stuff_id;

    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;

        ret.push_back(sqlite_orm_column("name", sqlite_orm_column::STRING, &name));
        ret.push_back(sqlite_orm_column("is_sale", sqlite_orm_column::INTEGER, &is_sale));
        ret.push_back(sqlite_orm_column("attachment", sqlite_orm_column::STRING, &attachment));
        ret.push_back(sqlite_orm_column("code", sqlite_orm_column::STRING, &code));
        ret.push_back(sqlite_orm_column("admin_name_phone", sqlite_orm_column::STRING, &admin_name_phone));
        ret.push_back(sqlite_orm_column("follow_stuff_id", sqlite_orm_column::STRING, &follow_stuff_id));
        ret.push_back(sqlite_orm_column("balance", sqlite_orm_column::REAL, &balance));
        ret.push_back(sqlite_orm_column("credit", sqlite_orm_column::REAL, &credit));

        return ret;
    }
    virtual std::string table_name()
    {
        return "contract_table";
    }
};
class sql_vehicle: public sql_tree_base
{
public:
    std::string plate_no;
    std::string back_plate_no;
    std::string driver_name;
    std::string driver_phone;
    std::string driver_id;
    long in_black_list = 0;
    long in_white_list = 0;

    virtual std::vector<sqlite_orm_column> self_columns_defined()
    {
        std::vector<sqlite_orm_column> ret;

        ret.push_back(sqlite_orm_column("plate_no", sqlite_orm_column::STRING, &plate_no));
        ret.push_back(sqlite_orm_column("back_plate_no", sqlite_orm_column::STRING, &back_plate_no));
        ret.push_back(sqlite_orm_column("driver_name", sqlite_orm_column::STRING, &driver_name));
        ret.push_back(sqlite_orm_column("driver_phone", sqlite_orm_column::STRING, &driver_phone));
        ret.push_back(sqlite_orm_column("driver_id", sqlite_orm_column::STRING, &driver_id));
        ret.push_back(sqlite_orm_column("in_black_list", sqlite_orm_column::INTEGER, &in_black_list));
        ret.push_back(sqlite_orm_column("in_white_list", sqlite_orm_column::INTEGER, &in_white_list));

        return ret;
    }
    virtual std::string table_name()
    {
        return "vehicle_table";
    }
};
std::unique_ptr<sql_user> db_get_online_user(const std::string &_token);

#endif