#if !defined(_ZH_RPC_UTIL_H_)
#define _ZH_RPC_UTIL_H_

#include "../zh_database/zh_db_config.h"
extern std::map<std::string,std::string> g_permisson_description_map;
#define ZH_RETURN_MSG(_msg)  do {gen_exp e;e.msg = _msg; throw e;} while (0)
#define ZH_RETURN_UNLOGIN_MSG() ZH_RETURN_MSG("用户未登录")
#define ZH_RETURN_NO_PRAVILIGE() ZH_RETURN_MSG("无权限")
#define ZH_RETURN_NEED_PRAVILIGE(_need_target)  do {ZH_RETURN_MSG("无权限，需要" + g_permisson_description_map[_need_target] + "权限");} while(0)
#define ZH_RETURN_DUP_CONTRACT() ZH_RETURN_MSG("合同已存在")
#define ZH_RETURN_NO_CONTRACT() ZH_RETURN_MSG("合同不存在")
#define ZH_RETURN_DUP_STUFF() ZH_RETURN_MSG("物料已存在")
#define ZH_RETURN_NO_STUFF() ZH_RETURN_MSG("物料不存在")
#define ZH_RETURN_DUP_VEHICLE() ZH_RETURN_MSG("车辆已存在")
#define ZH_RETURN_DUP_DRIVER() ZH_RETURN_MSG("司机重复绑定")
#define ZH_RETURN_NO_VEHICLE() ZH_RETURN_MSG("车辆不存在")
#define ZH_RETURN_DUP_ORDER() ZH_RETURN_MSG("派车内容已存在")
#define ZH_RETURN_NO_ORDER() ZH_RETURN_MSG("派车单不存在")
#define ZH_RETURN_ORDER_CANNOT_CANCEL(x) ZH_RETURN_MSG(x + " 无法取消")

#define ZH_RASTER_PORT 30200
#define ZH_SCALE_PORT 30201
#define ZH_ID_READER_PORT 30202
#define ZH_PRINTER_PORT 30203
#define ZH_QR_PORT 30204

struct zh_sub_callback_cfg {
    void *pData = nullptr;
    void (*callback)(const std::string &, const std::string &, void *);
    bool is_close = true;
};
#endif // _ZH_RPC_UTIL_H_
