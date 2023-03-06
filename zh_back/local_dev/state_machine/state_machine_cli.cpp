#include "lib/state_machine_control.h"

int main(int argc, char **argv)
{
    std::string cmd;
    std::string sm_name;
    std::string begin_date;
    std::string end_date;
    bool is_enter = false;
    using namespace clipp;
    auto cli = (command("tids").set(cmd) |
                (command("take_picture").set(cmd) & value("sm_name", sm_name)) & option("-e").set(is_enter) |
                (command("get_record").set(cmd) & value("sm_name", sm_name)) & option("-e").set(is_enter) & value("begin_date", begin_date)& value("end_date", end_date));
    if (!parse(argc, argv, cli))
    {
        std::cerr << "Usage:\n"
                  << usage_lines(cli, argv[0])
                  << '\n';
        return -1;
    }
    if (cmd == "take_picture")
    {
        std::cout << sm_control_take_picture(sm_name, is_enter) << std::endl;
    }
    else if (cmd == "get_record")
    {
        std::cout << sm_control_get_record(sm_name, is_enter, begin_date, end_date) << std::endl;
    }

    return 0;
}
