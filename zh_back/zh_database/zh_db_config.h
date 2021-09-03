#if !defined(_ZH_DB_CONFIG_H_)
#define _ZH_DB_CONFIG_H_

#include "sqlite_orm_tree.h"

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

class zh_sql_user_info:public sql_tree_base
{
public:
    std::string name;
    std::string phone;
    std::string password;
    long need_change_password = 0;

    zh_sql_user_info() {
        add_parent_type<zh_sql_user_permission>("permission");
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

class zh_sql_user_login:public sql_tree_base {
public:
    std::string ssid;
    long timestamp = 0;
    zh_sql_user_login() {
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

class zh_sql_file:public sql_tree_base {
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
    void save_file(const std::string &tmp_name, const std::string &new_name) {
        std::string prefix = "/manage_dist/logo_res/";
        int fd_orig = open(tmp_name.c_str(), O_RDONLY);
        int fd_new = open((prefix + new_name).c_str(), O_WRONLY|O_CREAT, S_IREAD|S_IWRITE);
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
class zh_sql_contract:public sql_tree_base {
public:
    std::string name;
    std::string date;
    long is_sale = 0;
    std::string code;
    zh_sql_contract() {
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

#endif // _ZH_DB_CONFIG_H_
