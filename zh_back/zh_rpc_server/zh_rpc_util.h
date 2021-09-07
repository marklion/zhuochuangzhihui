#if !defined(_ZH_RPC_UTIL_H_)
#define _ZH_RPC_UTIL_H_

#include "../zh_database/zh_db_config.h"

#define ZH_RETURN_MSG(_msg)  do {gen_exp e;e.msg = _msg; throw e;} while (0)
#define ZH_RETURN_UNLOGIN_MSG() ZH_RETURN_MSG("用户未登录")
#define ZH_RETURN_NO_PRAVILIGE() ZH_RETURN_MSG("无权限")
#define ZH_RETURN_DUP_CONTRACT() ZH_RETURN_MSG("合同已存在")
#define ZH_RETURN_NO_CONTRACT() ZH_RETURN_MSG("合同不存在")
#define ZH_RETURN_DUP_STUFF() ZH_RETURN_MSG("物料已存在")
#define ZH_RETURN_NO_STUFF() ZH_RETURN_MSG("物料不存在")
#define ZH_RETURN_DUP_VEHICLE() ZH_RETURN_MSG("车辆已存在")
#define ZH_RETURN_DUP_DRIVER() ZH_RETURN_MSG("司机重复绑定")
#define ZH_RETURN_NO_VEHICLE() ZH_RETURN_MSG("车辆不存在")

std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid);
std::unique_ptr<zh_sql_user_info> zh_rpc_util_get_online_user(const std::string &ssid, long required_permission);
std::string zh_rpc_util_gen_ssid();
std::string zh_rpc_util_get_timestring(time_t _time = time(NULL));
std::string zh_rpc_util_get_datestring(time_t _time = time(NULL));

#endif // _ZH_RPC_UTIL_H_
