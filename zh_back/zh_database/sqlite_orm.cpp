#include "sqlite_orm.h"

#include <iostream>
#include <sqlite3.h>
static tdf_log g_log("sql_execute");
extern bool execute_sql_cmd(const std::string &_sql_cmd, const std::string &_sql_file, std::vector<std::map<std::string, std::string>> *_ret)
{
    static sqlite3 *db = nullptr;
    bool ret = false;
    int sql_ret = -1;
    if (db)
    {
        sql_ret = 0;
    }
    else
    {
        sql_ret = sqlite3_open(_sql_file.c_str(), &db);
    }
    if (sql_ret == 0 && nullptr != db)
    {
        sqlite3_busy_handler(
            db, [](void *_priv, int _count) -> int
            {
            usleep(100000);
            if (_count>40)
            {
                g_log.err("max retry times arrived: %s", (char *)_priv);
                return 0;
            }
            g_log.log("retry sql :%s for %d times", (char *)_priv, _count);
            return 1; },
            (void *)(_sql_cmd.c_str()));
        char *errmsg = nullptr;
        if (0 ==
            sqlite3_exec(
                db,
                _sql_cmd.c_str(),
                [](void *_pQA, int argc, char **argv, char **_col) -> int
                {
                    auto pOut = static_cast<std::vector<std::map<std::string, std::string>> *>(_pQA);
                    if (pOut != nullptr)
                    {
                        std::map<std::string, std::string> tmp_map;
                        for (size_t i = 0; i < argc; i++)
                        {
                            if (argv[i] == nullptr)
                            {
                                tmp_map[_col[i]] = "";
                            }
                            else
                            {
                                tmp_map[_col[i]] = argv[i];
                            }
                        }
                        pOut->push_back(tmp_map);
                    }
                    return 0;
                },
                _ret, &errmsg))
        {
            ret = true;
            std::string output_log = "result of " + _sql_cmd + " is ";
            if (_ret)
            {
                if (_ret->size() > 0 && _sql_cmd.find("last_insert_rowid()") == _sql_cmd.npos && _sql_cmd.find("PRAGMA") == _sql_cmd.npos)
                {
                    output_log.append("\n");
                    bool header = true;
                    for (auto &itr : *(_ret))
                    {
                        if (header)
                        {
                            header = false;
                            auto map_itr = itr.begin();
                            for (; map_itr != itr.end(); ++map_itr)
                            {
                                output_log.append("| " + map_itr->first);
                            }
                            output_log.append("\n");
                        }
                        auto map_itr = itr.begin();
                        for (; map_itr != itr.end(); ++map_itr)
                        {
                            output_log.append("|" + map_itr->second);
                        }
                        output_log.append("\n");
                    }
                }
                else
                {
                    output_log.append("no thing");
                }
            }
            else
            {
                output_log.append("success");
            }
            g_log.log(output_log);
        }
        if (nullptr != errmsg)
        {
            g_log.err("meet error: %s when excute: %s", errmsg, _sql_cmd.c_str());
            sqlite3_free(errmsg);
        }
    }
    else
    {
        g_log.err("failed to open database: %s", _sql_file.c_str());
        db = nullptr;
    }

    return ret;
}

pthread_mutex_t sqlite_orm_lock::lock;

void sqlite_orm_lock::init_lock()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&sqlite_orm_lock::lock, &attr);
}

sqlite_orm_lock::sqlite_orm_lock()
{
    auto ret = pthread_mutex_lock(&sqlite_orm_lock::lock);
    if (0 == ret)
    {
        g_log.log("lock sql");
    }
    else
    {
        g_log.err("failed to lock sql");
    }
}
sqlite_orm_lock::~sqlite_orm_lock()
{
    pthread_mutex_unlock(&sqlite_orm_lock::lock);
    g_log.log("unlock sql");
}