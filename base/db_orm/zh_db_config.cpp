#include "zh_db_config.h"
#include <uuid/uuid.h>
#include <fstream>
#include <sstream>

std::unique_ptr<sql_user> db_get_online_user(const std::string &_token)
{
    if (_token.empty())
    {
        return std::unique_ptr<sql_user>();
    }
    return sqlite_orm::search_record<sql_user>("online_token == '%s'", _token.c_str());
}