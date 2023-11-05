#if !defined(_DEVICE_CLI_H_)
#define _DEVICE_CLI_H_


#include "../lib/rpc_include.h"

std::unique_ptr<cli::Menu> make_device_cli();

#endif // _DEVICE_CLI_H_
