#include "lib/zh_printer.h"

int main(int argc, char **argv)
{
	zh_printer_dev dev(argv[1]);
	dev.print_string(argv[2]);
}
