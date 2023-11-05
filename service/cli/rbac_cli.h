#if !defined(_RBAC_CLI_H)
#define _RBAC_CLI_H

#include "../lib/rpc_include.h"
std::unique_ptr<cli::Menu> make_rabc_cli();

#endif // _RBAC_CLI_H