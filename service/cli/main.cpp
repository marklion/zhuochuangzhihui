#include "../lib/rpc_include.h"
#include "rbac_cli.h"
#include "device_cli.h"

int main(int argc, char const *argv[])
{
    auto root_menu = std::unique_ptr<cli::Menu>(new cli::Menu("zczh"));

    root_menu->Insert(make_rabc_cli());
    root_menu->Insert(make_device_cli());

    cli::Cli cli(std::move(root_menu));
    cli::LoopScheduler sc;
    cli::CliLocalTerminalSession ss(cli, sc, std::cout);

    sc.Run();
    return 0;
}
