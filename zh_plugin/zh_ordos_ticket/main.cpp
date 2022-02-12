#include "lib/zh_ordos_ticket.h"
#include <unistd.h>
#include <iostream>
#include "clipp.h"

int main(int argc, char **argv)
{
    int iret = -1;
    std::string json_string;
    std::string key;
    std::string cmd;
    std::string user_name;
    std::string password;
    std::string verify_code;
    bool need_verify_code = false;
    using namespace clipp;
    auto cli = ((command("login").set(cmd) & (required("-g").set(need_verify_code) | required("-u") & value("username", user_name) & required("-p") & value("password", password) & required("-v") & value("verify_code", verify_code))) |
                (command("get").set(cmd) & required("-k") & value("json key", key)) |
                (command("set").set(cmd) & required("-k") & value("json key", key) & value("json value", json_string)) |
                command("init").set(cmd) |
                command("tids").set(cmd) |
                (command("finish").set(cmd) & value("req_json", json_string)) |
                (command("print").set(cmd) & value("req_json", json_string)));
    if (!parse(argc, argv, cli))
    {
        std::cerr << "Usage:\n"
                  << usage_lines(cli, argv[0])
                  << '\n';
    }

    if (cmd == "login")
    {
        if (need_verify_code)
        {
            auto verify_code = zh_ordos_ticket_get_verify_code();
            if (verify_code.length() > 0)
            {
                std::cout << verify_code << std::endl;
                iret = 0;
            }
        }
        else
        {
            if (zh_ordos_ticket_login(user_name, password, verify_code))
            {
                iret = 0;
            }
        }
    }
    else if (cmd == "get")
    {
        std::string get_value;
        auto config_value = zh_ordos_ticket_get_config()(key);
        if (!config_value.empty())
        {
            get_value = config_value;
        }
        else
        {
            get_value = zh_ordos_ticket_get_config()[key].ToString();
        }
        std::cout << get_value << std::endl;
        iret = 0;
    }
    else if (cmd == "set")
    {
        zh_ordos_set_config(key, json_string);
        iret = 0;
    }
    else if (cmd == "tids")
    {
        auto tids = zh_ordos_ticket_get_TIDs();
        if (!tids.empty())
        {
            std::cout << tids << std::endl;
            iret = 0;
        }
    }
    else if (cmd == "print")
    {
        if (zh_ordos_ticket_print_ticket(json_string))
        {
            iret = 0;
        }

    }
    else if (cmd == "init")
    {
        if (zh_ordos_ticket_init())
        {
            iret = 0;
        }
    }
    else if (cmd == "finish")
    {
        if (zh_ordos_ticket_proc_finish(json_string))
        {
            iret = 0;
        }
    }

    return iret;
}
