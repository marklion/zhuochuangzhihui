#include "config_management_imp.h"
#include "rpc_include.h"

config_management_handler::config_management_handler()
{
    struct driver_meta_tmp
    {
        std::string name;
        std::string path;
    } dmt_array[] = {
        {"stub_driver", "/bin/ls"}};

    for (auto &itr : dmt_array)
    {
        auto er = sqlite_orm::search_record<sql_device_driver>("name == '%s'", itr.name.c_str());
        if (!er)
        {
            sql_device_driver tmp;
            tmp.name = itr.name;
            tmp.path = itr.path;
            tmp.insert_record();
        }
    }
}

void config_management_handler::get_stuff_config(std::vector<stuff_config> &_return)
{
    auto all_record = sqlite_orm::search_record_all<sql_stuff>();
    for (auto &itr : all_record)
    {
        stuff_config tmp;
        tmp.id = itr.get_pri_id();
        tmp.stuff_name = itr.name;

        _return.push_back(tmp);
    }
}
bool config_management_handler::add_stuff_config(const stuff_config &new_one)
{
    bool ret = false;

    sql_stuff tmp;
    tmp.name = new_one.stuff_name;

    ret = tmp.insert_record();

    return ret;
}
bool config_management_handler::del_stuff_config(const int64_t id)
{
    bool ret = false;

    auto ss = sqlite_orm::search_record<sql_stuff>(id);
    if (ss)
    {
        ss->remove_record();
        ret = true;
    }

    return ret;
}

void config_management_handler::get_scale_config(std::vector<device_scale_set> &_return)
{
    auto gs = sqlite_orm::search_record_all<sql_device_set>("is_scale != 0");
    for (auto &itr : gs)
    {
        device_scale_set dgs;
        db_2_rpc(itr, dgs);
        _return.push_back(dgs);
    }
}

void config_management_handler::get_gate_config(std::vector<device_gate_set> &_return)
{
    auto gs = sqlite_orm::search_record_all<sql_device_set>("is_scale == 0");
    for (auto &itr : gs)
    {
        device_gate_set dgs;
        db_2_rpc(itr, dgs);
        _return.push_back(dgs);
    }
}

void config_management_handler::get_all_driver(std::vector<device_driver> &_return)
{
    auto ds = sqlite_orm::search_record_all<sql_device_driver>();
    for (auto &itr : ds)
    {
        device_driver tmp;
        db_2_rpc(itr, tmp);
        _return.push_back(tmp);
    }
}

bool config_management_handler::add_device_to_set(const std::string &name, const std::string &driver_args, const int64_t driver_id, const int64_t set_id, const std::string &use_for)
{
    bool ret = false;
    sql_device_meta dev_m;
    dev_m.name = name;
    dev_m.args = driver_args;

    auto driver = sqlite_orm::search_record<sql_device_driver>(driver_id);
    auto set = sqlite_orm::search_record<sql_device_set>(set_id);
    if (driver && set)
    {
        if (!set->get_parent<sql_device_meta>(use_for))
        {
            dev_m.set_parent(*driver, "driver");
            if (dev_m.insert_record())
            {
                set->set_parent(dev_m, use_for);
                ret = set->update_record();
            }
        }
        else
        {
            ZH_RETURN_MSG("设备已存在");
        }
    }
    else
    {
        ZH_RETURN_MSG("驱动或组件不存在");
    }

    return ret;
}

bool config_management_handler::del_device_from_set(const int64_t device_id)
{
    bool ret = true;

    auto dev = sqlite_orm::search_record<sql_device_meta>(device_id);
    if (dev)
    {
        dev->remove_record();
    }
    else
    {
        ZH_RETURN_MSG("设备不存在");
    }

    return ret;
}

bool config_management_handler::add_device_set(const std::string &name, const bool is_scale)
{
    bool ret = false;

    auto er = sqlite_orm::search_record<sql_device_set>("name == '%s'", name.c_str());
    if (er)
    {
        ZH_RETURN_MSG("组件已存在");
    }
    sql_device_set tmp;
    tmp.is_scale = is_scale;
    tmp.name = name;

    ret = tmp.insert_record();

    return ret;
}

bool config_management_handler::del_device_set(const int64_t set_id)
{
    bool ret = false;

    auto er = sqlite_orm::search_record<sql_device_set>(set_id);
    if (!er)
    {
        ZH_RETURN_MSG("组件不存在");
    }
    if (!er->is_empty_set())
    {
        ZH_RETURN_MSG("组件已经添加设备，请先删除设备");
    }
    er->remove_record();
    ret = true;

    return ret;
}

void config_management_handler::db_2_rpc(sql_device_driver &_db, device_driver &_rpc)
{
    _rpc.id = _db.get_pri_id();
    _rpc.name = _db.name;
    _rpc.path = _db.path;
}

void config_management_handler::db_2_rpc(sql_device_meta &_db, device_meta &_rpc)
{
    auto driver = _db.get_parent<sql_device_driver>("driver");
    if (driver)
    {
        db_2_rpc(*driver, _rpc.driver);
    }
    _rpc.driver_args = _db.args;
    _rpc.id = _db.get_pri_id();
    _rpc.name = _db.name;
}

#define DEV_FROM_SET_TO_RPC(x, y)                 \
    auto x = _db.get_parent<sql_device_meta>(#x); \
    if (x)                                        \
    {                                             \
        db_2_rpc(*x, y);                          \
    }

void config_management_handler::db_2_rpc(sql_device_set &_db, device_scale_set &_rpc)
{
    _rpc.name = _db.name;
    _rpc.id = _db.get_pri_id();
    DEV_FROM_SET_TO_RPC(front_gate, _rpc.gate.front);
    DEV_FROM_SET_TO_RPC(back_gate, _rpc.gate.back);
    DEV_FROM_SET_TO_RPC(front_plate_cam, _rpc.plate_cam.front);
    DEV_FROM_SET_TO_RPC(back_plate_cam, _rpc.plate_cam.back);
    DEV_FROM_SET_TO_RPC(front_video_cam, _rpc.video_cam.front);
    DEV_FROM_SET_TO_RPC(back_video_cam, _rpc.video_cam.back);
    DEV_FROM_SET_TO_RPC(front_id_reader, _rpc.id_reader.front);
    DEV_FROM_SET_TO_RPC(back_id_reader, _rpc.id_reader.back);
    DEV_FROM_SET_TO_RPC(front_qr_reader, _rpc.qr_reader.front);
    DEV_FROM_SET_TO_RPC(back_qr_reader, _rpc.qr_reader.back);
    DEV_FROM_SET_TO_RPC(front_led, _rpc.led.front);
    DEV_FROM_SET_TO_RPC(back_led, _rpc.led.back);
    DEV_FROM_SET_TO_RPC(front_speaker, _rpc.speaker.front);
    DEV_FROM_SET_TO_RPC(back_speaker, _rpc.speaker.back);
    DEV_FROM_SET_TO_RPC(front_printer, _rpc.printer.front);
    DEV_FROM_SET_TO_RPC(back_printer, _rpc.printer.back);
    DEV_FROM_SET_TO_RPC(scale, _rpc.scale);
}

void config_management_handler::db_2_rpc(sql_device_set &_db, device_gate_set &_rpc)
{
    _rpc.name = _db.name;
    _rpc.id = _db.get_pri_id();
    DEV_FROM_SET_TO_RPC(front_gate, _rpc.gate.front);
    DEV_FROM_SET_TO_RPC(back_gate, _rpc.gate.back);
    DEV_FROM_SET_TO_RPC(front_plate_cam, _rpc.plate_cam.front);
    DEV_FROM_SET_TO_RPC(back_plate_cam, _rpc.plate_cam.back);
    DEV_FROM_SET_TO_RPC(front_video_cam, _rpc.video_cam.front);
    DEV_FROM_SET_TO_RPC(back_video_cam, _rpc.video_cam.back);
    DEV_FROM_SET_TO_RPC(front_id_reader, _rpc.id_reader.front);
    DEV_FROM_SET_TO_RPC(back_id_reader, _rpc.id_reader.back);
    DEV_FROM_SET_TO_RPC(front_qr_reader, _rpc.qr_reader.front);
    DEV_FROM_SET_TO_RPC(back_qr_reader, _rpc.qr_reader.back);
    DEV_FROM_SET_TO_RPC(front_led, _rpc.led.front);
    DEV_FROM_SET_TO_RPC(back_led, _rpc.led.back);
    DEV_FROM_SET_TO_RPC(front_speaker, _rpc.speaker.front);
    DEV_FROM_SET_TO_RPC(back_speaker, _rpc.speaker.back);
}
