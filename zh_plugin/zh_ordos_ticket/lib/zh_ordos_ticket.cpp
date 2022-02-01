#include "zh_ordos_ticket.h"
#include <iostream>

bool zh_ordos_ticket_init()
{
    std::cout << "init" << std::endl;
    return true;
}
void zh_ordos_ticket_fini()
{
    std::cout << "fini" << std::endl;
}
void zh_ordos_ticket_order_finish(const std::string &_msg)
{
    std::cout << "proc order finish,msg:" << _msg << std::endl;
}
void zh_ordos_ticket_proc_return(const std::string &_msg)
{
    std::cout << "proc return,msg:" << _msg << std::endl;
}