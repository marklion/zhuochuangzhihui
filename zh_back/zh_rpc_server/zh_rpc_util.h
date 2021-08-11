#if !defined(_ZH_RPC_UTIL_H_)
#define _ZH_RPC_UTIL_H_

#include "../zh_database/zh_db_config.h"

#define ZH_RETURN_MSG(_msg)  do {gen_exp e;e.msg = _msg; throw e;} while (0)
#define ZH_RETURN_UNLOGIN_MSG() ZH_RETURN_MSG("用户未登录")

std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid);
std::string zh_rpc_util_gen_ssid();

#endif // _ZH_RPC_UTIL_H_
