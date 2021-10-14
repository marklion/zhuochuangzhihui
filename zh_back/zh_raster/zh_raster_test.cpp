#include "lib/zh_raster.h"

int main(int argc, char const *argv[])
{
    if (raster_was_block(argv[1], atoi(argv[2])))
    {
        puts("block");
    }
    else
    {
        puts("clear");
    }

    return 0;
}
