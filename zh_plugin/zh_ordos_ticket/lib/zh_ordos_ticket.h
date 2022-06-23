#if !defined(_ZH_ORDOS_TICKETS_H_)
#define _ZH_ORDOS_TICKETS_H_
#include <string>
#include <list>
#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../../zh_pub/zh_cpp_pub/Base64.h"

std::string zh_ordos_ticket_get_TIDs();
std::string zh_ordos_ticket_get_verify_code();
bool zh_ordos_ticket_login(const std::string &_username, const std::string &_password, const std::string &_verify_code);
void zh_ordos_ticket_order_finish(const std::string &_msg);
bool zh_ordos_ticket_print_ticket(const std::string &_msg);
bool zh_ordos_ticket_proc_finish(const std::string &_msg);
bool zh_ordos_ticket_init();
void zh_ordos_ticket_refresh();
#endif // _ZH_ORDOS_TICKETS_H_
