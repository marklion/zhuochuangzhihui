#include "lib/zh_traffic_light.h"
int main(int argc, char const *argv[])
{
    std::string ip(argv[2]);
    if (std::string(argv[1]) == "1")
    {
        ZH_TRLI_set_green(ip);
    }
    else
    {
        ZH_TRLI_set_red(ip);
    }
    return 0;
}
