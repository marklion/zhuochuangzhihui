#include "../lib/rpc_include.h"
#include "rbac_cli.h"
#include "device_cli.h"
#include <fstream>


int main(int argc, char const *argv[])
{
    common_cli *sub_c[] = {
        new rbac_cli(),new device_cli()
    };
    auto root_menu = std::unique_ptr<cli::Menu>(new cli::Menu("zczh"));
    for (auto &itr:sub_c)
    {
        root_menu->Insert(std::move(itr->menu));
    }

    root_menu->Insert("bdr", [&](std::ostream &_out){
        for (auto &itr:sub_c)
        {
            _out << itr->menu_name << std::endl;
            _out << itr->make_bdr() << std::endl;
            _out << "zczh" << std::endl;
        }
    });

    cli::Cli cli(std::move(root_menu));
    if (argc == 1)
    {
        cli::LoopScheduler sc;
        cli::CliLocalTerminalSession ss(cli, sc, std::cout);

        sc.Run();
    }
    else
    {
        try
        {
            std::fstream cmd_file(argv[1], std::ios::in);
            cli::CliFileSession cf(cli, cmd_file);
            cf.Start();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

    }

    return 0;
}
