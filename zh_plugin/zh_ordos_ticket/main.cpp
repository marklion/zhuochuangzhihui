#include "lib/zh_ordos_ticket.h"
#include <unistd.h>
#include <iostream>

int main(int argc, char *const *argv)
{
    int o;
    const char *optstring = "ifop";
    while ((o = getopt(argc, argv, optstring)) != -1)
    {
        switch (o)
        {
        case 'i':
            zh_ordos_ticket_init();
            break;
        case 'f':
            zh_ordos_ticket_fini();
            break;
        case 'o':
        {
            std::string msg;
            std::cin >> msg;
            zh_ordos_ticket_order_finish(msg);
            break;
        }
        case 'p':
        {
            std::string msg;
            std::cin >> msg;
            zh_ordos_ticket_proc_return(msg);
            break;
        }
        }
    }
    return 0;
}
