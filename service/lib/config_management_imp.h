#if !defined(_CONFIG_MANAGEMENT_H_)
#define _CONFIG_MANAGEMENT_H_

#include "../../base/include.h"

class config_management_handler : public config_managementIf
{
public:
    virtual void get_stuff_config(std::vector<stuff_config> &_return);
    virtual bool add_stuff_config(const stuff_config &new_one);
    virtual bool del_stuff_config(const int64_t id);
};
#endif // _CONFIG_MANAGEMENT_H_