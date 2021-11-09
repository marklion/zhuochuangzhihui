#include "lib/zh_scale.h"
#include <iostream>

int main(int argc, char const *argv[])
{
    std::cout << get_current_weight(argv[1], atoi(argv[2])) << std::endl;
    return 0;
}
