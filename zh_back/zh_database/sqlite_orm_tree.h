#if !defined(_SQLITE_ORM_TREE_H_)
#define _SQLITE_ORM_TREE_H_

#include "sqlite_orm.h"
#include <map>
#include <algorithm>

struct parent_info
{
    std::string parent_table;
    long parent_id = 0;
    parent_info() {}
    parent_info(const std::string &_table, long _id) : parent_table(_table), parent_id(_id) {}
};

class sql_tree_base : public sqlite_orm
{
    std::map<std::string, parent_info> m_parent;

public:
    virtual std::vector<sqlite_orm_column> self_columns_defined() = 0;
    sql_tree_base() : sqlite_orm("/database/zh.db") {}
    virtual std::vector<sqlite_orm_column> columns_defined()
    {
        std::vector<sqlite_orm_column> ret(self_columns_defined());
        auto itr = m_parent.begin();
        for (; itr != m_parent.end(); itr++)
        {
            ret.push_back(sqlite_orm_column(itr->first + "_ext_key", sqlite_orm_column::INTEGER, &(itr->second.parent_id)));
        }

        return ret;
    }
    template <typename T>
    void add_parent_type(const std::string &_parent_tag)
    {
        T tmp_parent;
        auto parent_table_name = tmp_parent.table_name();
        parent_info pi(parent_table_name, 0);
        m_parent[_parent_tag] = pi;
    }
    bool set_parent(sql_tree_base &_parent_record, const std::string &_parent_tag)
    {
        bool ret = false;
        auto parent_table_name = _parent_record.table_name();
        auto record_id = _parent_record.get_pri_id();
        auto parent_record_itr = m_parent.find(_parent_tag);
        if (parent_record_itr != m_parent.end())
        {
            parent_record_itr->second.parent_table = parent_table_name;
            parent_record_itr->second.parent_id = record_id;
            ret = true;
        }

        return ret;
    }
    template <typename T>
    std::unique_ptr<T> get_parent(const std::string &_parent_tag)
    {
        T tmp;
        auto parent_table_name = tmp.table_name();
        auto parent_record_itr = m_parent.find(_parent_tag);
        if (parent_record_itr != m_parent.end())
        {
            auto parent_id = parent_record_itr->second.parent_id;
            return sqlite_orm::search_record<T>(parent_id);
        }

        return std::unique_ptr<T>();
    }

    template <typename T>
    std::list<T> get_all_children(const std::string &_parent_tag)
    {
        return sqlite_orm::search_record_all<T>("%s_ext_key = %d", _parent_tag.c_str(), get_pri_id());
    }
    template <typename T>
    std::list<T> get_all_children(const std::string &_parent_tag, const char *_query, ...)
    {
        va_list vl;
        va_start(vl, _query);
        char tmpbuff[2048];
        vsnprintf(tmpbuff, sizeof(tmpbuff), _query, vl);
        va_end(vl);
        return sqlite_orm::search_record_all<T>("%s_ext_key = %d AND %s", _parent_tag.c_str(), get_pri_id(), tmpbuff);
    }
    template <typename T>
    std::unique_ptr<T> get_children(const std::string &_parent_tag)
    {
        auto all_children = sqlite_orm::search_record_all<T>("%s_ext_key = %d", _parent_tag.c_str(), get_pri_id());

        if (all_children.size() > 0)
        {
            return std::unique_ptr<T>(new T(all_children.front()));
        }
        return std::unique_ptr<T>();
    }
    template <typename T>
    std::unique_ptr<T> get_children(const std::string &_parent_tag, const char *_query, ...)
    {
        va_list vl;
        va_start(vl, _query);
        char tmpbuff[2048];
        vsnprintf(tmpbuff, sizeof(tmpbuff), _query, vl);
        va_end(vl);

        return sqlite_orm::search_record<T>("%s_ext_key = %d AND %s",  _parent_tag.c_str(), get_pri_id(), tmpbuff);
    }
};

#endif // _SQLITE_ORM_TREE_H_
