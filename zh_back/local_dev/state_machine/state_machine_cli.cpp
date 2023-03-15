#include "lib/state_machine_control.h"

int main(int argc, char **argv)
{
    std::string cmd;
    std::string sm_name;
    std::string begin_date;
    std::string end_date;
    bool is_enter = false;
    bool is_open = false;
    std::string plate;
    std::string content;
    using namespace clipp;
    auto cli = (command("all").set(cmd) |
                (command("take_picture").set(cmd) & value("sm_name", sm_name), option("-e").set(is_enter)) |
                (command("open_gate").set(cmd) , value("sm_name", sm_name) , option("-e").set(is_enter) , (option("-o").set(is_open))) |
                (command("traffic").set(cmd) , value("sm_name", sm_name) , option("-e").set(is_enter) , (option("-g").set(is_open))) |
                (command("trigger").set(cmd) , value("sm_name", sm_name) , opt_value("plate", plate) , option("-e").set(is_enter)) |
                (command("led_show").set(cmd) , value("sm_name", sm_name) , opt_value("content", content) , option("-e").set(is_enter)) |
                (command("print").set(cmd) , value("sm_name", sm_name) , opt_value("content", content), opt_value("qr_content", plate) , option("-e").set(is_enter)) |
                command("reset").set(cmd) & value("sm_name", sm_name) |
                command("confirm_weight").set(cmd) & value("sm_name", sm_name) |
                (command("get_record").set(cmd) & value("sm_name", sm_name) & value("begin_date", begin_date) & value("end_date", end_date), option("-e").set(is_enter) ));
    auto parse_result = parse(argc, argv, cli);
    if (!parse_result)
    {
        std::cout << make_man_page(cli, argv[0]);
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
    else if (cmd == "all")
    {
        auto all_sm = sm_control_get_all_sm();
        for (auto &itr : all_sm)
        {
            std::cout << "**************************" << std::endl;
            std::cout << itr.sm_type << ":" << itr.name << std::endl;
            std::cout << "入口道闸：" << (itr.enter_gate_is_close ? "关" : "开") << "|出口道闸:" << (itr.exit_gate_is_close ? "关" : "开") << std::endl;
            std::cout << "当前状态：" << itr.status_string << std::endl;
            std::cout << "当前重量：" << itr.cur_weight << std::endl;
            std::cout << "**************************" << std::endl;
        }
    }
    else if (cmd == "open_gate")
    {
        sm_control_open_gate(sm_name, is_enter, is_open);
    }
    else if (cmd == "trigger")
    {
        sm_control_trigger(sm_name, is_enter, plate);
    }
    else if (cmd == "reset")
    {
        sm_control_reset(sm_name);
    }
    else if (cmd == "confirm_weight")
    {
        sm_control_confirm_weight(sm_name);
    }
    else if (cmd == "print")
    {
        sm_control_print(sm_name, is_enter, content, plate);
    }
    else if (cmd == "led_show")
    {
        sm_control_led_cast(sm_name, is_enter, content);
    }
    else if (cmd == "traffic")
    {
        sm_control_set_traffic_light(sm_name, is_enter, is_open);
    }

    return 0;
}
