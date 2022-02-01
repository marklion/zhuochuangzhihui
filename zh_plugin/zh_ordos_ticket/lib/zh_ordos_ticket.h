#if !defined(_ZH_ORDOS_TICKETS_H_)
#define _ZH_ORDOS_TICKETS_H_
#include <string>

bool zh_ordos_ticket_init();
void zh_ordos_ticket_fini();
void zh_ordos_ticket_order_finish(const std::string &_msg);
void zh_ordos_ticket_proc_return(const std::string &_msg);

#endif // _ZH_ORDOS_TICKETS_H_
