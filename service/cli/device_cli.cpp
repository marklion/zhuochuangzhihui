#include "device_cli.h"
#include "tabulate.hpp"

void show_device_driver(std::ostream &out, std::vector<std::string> _params)
{
    std::vector<device_driver> tmp;
    try
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        client->get_all_driver(tmp);
        TRH_CLOSE();
        tabulate::Table tab;
        tab.add_row({"ID", "name"});
        for (auto &itr : tmp)
        {
            tab.add_row({std::to_string(itr.id), itr.name});
        }
        out << tab << std::endl;
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void add_set(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 2)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(config_management);
            THR_CONNECT(config_management);
            client->add_device_set(_params[0], atoi(_params[1].c_str()));
            TRH_CLOSE();
        }
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void del_set(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 1)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(config_management);
            THR_CONNECT(config_management);
            client->del_device_set(atoi(_params[0].c_str()));
            TRH_CLOSE();
        }
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

#define PUT_DEVICE_TO_SET(x)                                                \
    if (itr.x.id > 0)                                                       \
    {                                                                       \
        device_info.push_back(std::to_string(itr.x.id) + ":" + itr.x.name); \
    }

void show_device_scale(std::ostream &out, std::vector<std::string> _params)
{
    std::vector<device_scale_set> tmp;
    try
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        client->get_scale_config(tmp);
        TRH_CLOSE();
        tabulate::Table tab;
        tab.add_row({"ID", "name", "devices"});
        for (auto &itr : tmp)
        {
            std::vector<std::string> device_info;
            PUT_DEVICE_TO_SET(plate_cam.front);
            PUT_DEVICE_TO_SET(plate_cam.back);
            PUT_DEVICE_TO_SET(video_cam.front);
            PUT_DEVICE_TO_SET(video_cam.back);
            PUT_DEVICE_TO_SET(led.front);
            PUT_DEVICE_TO_SET(led.back);
            PUT_DEVICE_TO_SET(speaker.front);
            PUT_DEVICE_TO_SET(speaker.back);
            PUT_DEVICE_TO_SET(gate.front);
            PUT_DEVICE_TO_SET(gate.back);
            PUT_DEVICE_TO_SET(id_reader.front);
            PUT_DEVICE_TO_SET(id_reader.back);
            PUT_DEVICE_TO_SET(qr_reader.front);
            PUT_DEVICE_TO_SET(qr_reader.back);
            PUT_DEVICE_TO_SET(printer.front);
            PUT_DEVICE_TO_SET(printer.back);
            PUT_DEVICE_TO_SET(scale);
            tab.add_row({std::to_string(itr.id), itr.name, util_join_string(device_info, "\n")});
        }
        out << tab << std::endl;
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}
void show_device_gate(std::ostream &out, std::vector<std::string> _params)
{
    std::vector<device_gate_set> tmp;
    try
    {
        THR_DEF_CIENT(config_management);
        THR_CONNECT(config_management);
        client->get_gate_config(tmp);
        TRH_CLOSE();
        tabulate::Table tab;
        tab.add_row({"ID", "name", "devices"});
        for (auto &itr : tmp)
        {
            std::vector<std::string> device_info;
            PUT_DEVICE_TO_SET(plate_cam.front);
            PUT_DEVICE_TO_SET(plate_cam.back);
            PUT_DEVICE_TO_SET(video_cam.front);
            PUT_DEVICE_TO_SET(video_cam.back);
            PUT_DEVICE_TO_SET(led.front);
            PUT_DEVICE_TO_SET(led.back);
            PUT_DEVICE_TO_SET(speaker.front);
            PUT_DEVICE_TO_SET(speaker.back);
            PUT_DEVICE_TO_SET(gate.front);
            PUT_DEVICE_TO_SET(gate.back);
            PUT_DEVICE_TO_SET(id_reader.front);
            PUT_DEVICE_TO_SET(id_reader.back);
            PUT_DEVICE_TO_SET(qr_reader.front);
            PUT_DEVICE_TO_SET(qr_reader.back);
            tab.add_row({std::to_string(itr.id), itr.name, util_join_string(device_info, "\n")});
        }
        out << tab << std::endl;
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}
void add_device(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 5)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(config_management);
            THR_CONNECT(config_management);
            client->add_device_to_set(_params[0], _params[1], atoi(_params[2].c_str()), atoi(_params[3].c_str()), _params[4]);
            TRH_CLOSE();
        }
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}
void del_device(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 1)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(config_management);
            THR_CONNECT(config_management);
            client->del_device_from_set(atoi(_params[0].c_str()));
            TRH_CLOSE();
        }
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}

void start_device(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 1)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(device_management);
            THR_CONNECT_DM(device_management);
            client->device_ctrl(atoi(_params[0].c_str()), 1);
            TRH_CLOSE();
        }
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}
void stop_device(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 1)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(device_management);
            THR_CONNECT_DM(device_management);
            client->device_ctrl(atoi(_params[0].c_str()), 0);
            TRH_CLOSE();
        }
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}
void show_device_status(std::ostream &out, std::vector<std::string> _params)
{
    try
    {
        if (_params.size() != 1)
        {
            out << "参数错误" << std::endl;
        }
        else
        {
            THR_DEF_CIENT(device_management);
            THR_CONNECT_DM(device_management);
            auto is_started = client->device_is_started(atoi(_params[0].c_str()));
            if (is_started)
            {
                out << "启动" << std::endl;
            }
            else
            {
                out << "关闭" << std::endl;
            }
            TRH_CLOSE();
        }
    }
    catch (gen_exp e)
    {
        out << e.msg << std::endl;
    }
}



std::unique_ptr<cli::Menu> make_device_cli()
{
    auto root_menu = std::unique_ptr<cli::Menu>(new cli::Menu("device"));

    root_menu->Insert(CLI_MENU_ITEM(show_device_driver), "查看驱动");
    root_menu->Insert(CLI_MENU_ITEM(show_device_gate), "查看门组件");
    root_menu->Insert(CLI_MENU_ITEM(show_device_scale), "查看磅组件");
    root_menu->Insert(CLI_MENU_ITEM(add_set), "添加组件", {"组件名", "类型：0->门,1->磅"});
    root_menu->Insert(CLI_MENU_ITEM(del_set), "删除组件", {"组件编号"});
    root_menu->Insert(CLI_MENU_ITEM(add_device), "添加设备", {"设备名", "参数", "驱动编号", "组件编号", "用途"});
    root_menu->Insert(CLI_MENU_ITEM(del_device), "删除设备", {"设备编号"});
    root_menu->Insert(CLI_MENU_ITEM(start_device), "启动设备", {"设备编号"});
    root_menu->Insert(CLI_MENU_ITEM(stop_device), "关闭设备", {"设备编号"});
    root_menu->Insert(CLI_MENU_ITEM(show_device_status), "查看设备启动状态", {"设备编号"});

    return root_menu;
}
