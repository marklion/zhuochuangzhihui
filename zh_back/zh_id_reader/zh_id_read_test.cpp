#include "lib/zh_id_reader.h"

int main(int argc, char const *argv[])
{
    std::cout << read_id_no(argv[1], atoi(argv[2])) << std::endl;
    return 0;
}
