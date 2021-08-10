#include "system_management_imp.h"

system_management_handler *system_management_handler::m_inst = nullptr;

bool system_management_handler::reboot_system(const std::string &ssid)
{
    return true;
}
void system_management_handler::current_version(std::string &_return)
{
    _return = "v1.0";
}