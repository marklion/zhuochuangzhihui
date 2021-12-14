#if !defined(_ZHQR_LIB_H_)
#define _ZHQR_LIB_H_

#include "../../zh_rpc_server/zh_rpc_util.h"
#include "../../zh_tdf/tdf_include.h"

bool zh_qr_subscribe(const std::string &_ip, zh_sub_callback_cfg _cfg);
void zh_qr_unsubscribe(const std::string &_ip);


#endif // _ZHQR_LIB_H_
