#include "vehicle_order_center_imp.h"
#include "zh_rpc_util.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"
#include "../zh_id_reader/lib/zh_id_reader.h"
#include "system_management_imp.h"
#include "../zh_raster/lib/zh_raster.h"

vehicle_order_center_handler *vehicle_order_center_handler::m_inst = nullptr;

void vehicle_order_center_handler::get_order_by_anchor(std::vector<vehicle_order_info> &_return, const std::string &ssid, const int64_t anchor)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    std::string contract_query = "PRI_ID != 0";
    auto contract = opt_user->get_parent<zh_sql_contract>("belong_contract");
    if (contract)
    {
        contract_query = "company_name == '" + contract->name + "'";
    }

    auto orders = sqlite_orm::search_record_all<zh_sql_vehicle_order>("%s ORDER BY PRI_ID DESC LIMIT 30 OFFSET %ld", contract_query.c_str() , anchor);
    for (auto &itr:orders)
    {
        vehicle_order_info tmp;
        tmp.behind_vehicle_number = itr.behind_vehicle_number;
        tmp.driver_id = itr.driver_id;
        tmp.driver_name = itr.driver_name;
        tmp.driver_phone = itr.driver_phone;
        tmp.id = itr.get_pri_id();
        tmp.main_vehicle_number = itr.main_vehicle_number;
        tmp.order_number = itr.order_number;
        tmp.status = itr.status;
        tmp.stuff_name = itr.stuff_name;
        tmp.company_name = itr.company_name;
        auto all_status = itr.get_all_children<zh_sql_order_status>("belong_order");
        for (auto &single_status:all_status)
        {
            order_status_info status_detail;
            status_detail.name = single_status.name;
            status_detail.step = single_status.step;
            status_detail.timestamp = single_status.timestamp;
            status_detail.user_name = single_status.user_name;
            tmp.status_details.push_back(status_detail);
        }

        _return.push_back(tmp);
    }
}
void vehicle_order_center_handler::get_gate_info(gate_relate_info &_return, const std::string &ssid, const int64_t order_id)
{
}
void vehicle_order_center_handler::get_weight_info(weight_relate_info &_return, const std::string &ssid, const int64_t order_id)
{
}

bool vehicle_order_is_dup(const vehicle_order_info &order)
{
    bool ret = false;

    auto exist_record = sqlite_orm::search_record<zh_sql_vehicle_order>("PRI_ID != %ld AND status != 100 AND (main_vehicle_number == '%s' OR behind_vehicle_number == '%s' OR driver_id == '%s')", order.id, order.main_vehicle_number.c_str(), order.behind_vehicle_number.c_str(), order.driver_id.c_str());
    if (exist_record)
    {
        ret = true;
    }

    return ret;
}

bool vehicle_order_center_handler::create_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &orders)
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        if (orders.size() > 0)
        {
            auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", orders[0].company_name.c_str());
            opt_user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
        }
    }
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    for (auto &order : orders)
    {
        if (vehicle_order_is_dup(order))
        {
            ZH_RETURN_DUP_ORDER();
        }
    }

    for (auto &order : orders)
    {
        zh_sql_vehicle_order tmp;
        tmp.behind_vehicle_number = order.behind_vehicle_number;
        tmp.driver_id = order.driver_id;
        tmp.driver_name = order.driver_name;
        tmp.driver_phone = order.driver_phone;
        tmp.main_vehicle_number = order.main_vehicle_number;
        tmp.stuff_name = order.stuff_name;
        tmp.company_name = order.company_name;
        tmp.status = -1;

        auto contract = opt_user->get_parent<zh_sql_contract>("belong_contract");
        bool op_permit = true;
        if (contract)
        {
            if (contract->name != tmp.company_name)
            {
                op_permit = false;
            }
        }
        if (op_permit && tmp.insert_record())
        {
            tmp.order_number = std::to_string(time(nullptr)) + std::to_string(tmp.get_pri_id());
            auto create_status = zh_sql_order_status::make_create_status(ssid);
            tmp.push_status(create_status);
            if (!opt_user->get_parent<zh_sql_contract>("belong_contract"))
            {
                auto before_come_status = zh_sql_order_status::make_before_come_status();
                tmp.push_status(before_come_status);
            }
            ret = tmp.update_record();
        }
    }

    return ret;
}

bool vehicle_order_center_handler::confirm_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order)
{
    bool ret = true;
    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);

    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    for (auto &itr : order)
    {
        auto single_order = sqlite_orm::search_record<zh_sql_vehicle_order>(itr.id);
        if (single_order)
        {
            auto tmp = zh_sql_order_status::make_before_come_status(ssid);
            single_order->push_status(tmp);
        }
    }

    return ret;
}

bool vehicle_order_center_handler::cancel_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order)
{
    bool ret = true;

    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        if (order.size() > 0)
        {
            auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", order[0].company_name.c_str());
            opt_user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
        }
    }
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    std::list<zh_sql_vehicle_order> need_cancel;
    for (auto &itr:order)
    {
        auto single_order = sqlite_orm::search_record<zh_sql_vehicle_order>(itr.id);
        if (!single_order || single_order->status == 2)
        {
            ZH_RETURN_ORDER_CANNOT_CANCEL(itr.main_vehicle_number);
        }
        need_cancel.push_back(*single_order);
    }

    for (auto &itr:need_cancel)
    {
        auto cancel_status = zh_sql_order_status::make_end_status(ssid);
        itr.push_status(cancel_status);
    }

    return ret;
}

std::unique_ptr<tdf_state_machine_state> vehicle_sm_enter_scale::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.vehicle_on_scale())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_do_scale());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_enter_scale::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.record_cur_scale();
}
void vehicle_sm_enter_scale::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.open_door();
}
void vehicle_sm_enter_scale::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.close_door();
}
std::string vehicle_sm_enter_scale::state_name()
{
    return "上磅";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_exit_gate::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.vehicle_pass_gate())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_end());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_exit_gate::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.record_exit_time();
}
void vehicle_sm_exit_gate::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.show_exit_info();
    vsm.open_door();
}
void vehicle_sm_exit_gate::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.clean_show_info();
    vsm.close_door();
}
std::string vehicle_sm_exit_gate::state_name()
{
    return "出厂";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_before_enter_gate::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.can_enter())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_enter_gate());
    }
    else if (vsm.cannot_enter())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_end());
    }
    else if (vsm.enter_timeout())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_end());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_before_enter_gate::do_action(tdf_state_machine &_sm)
{
}
void vehicle_sm_before_enter_gate::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.start_enter_timer();
    vsm.show_before_enter_info();
}
void vehicle_sm_before_enter_gate::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.close_all_timer();
    vsm.clean_show_info();
}
std::string vehicle_sm_before_enter_gate::state_name()
{
    return "进门前";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_insite::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.gate_exit_near())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_before_exit_gate());
    }
    else if (vsm.scale_entry_near())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_before_scale());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_insite::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.vehicle_save();
}
void vehicle_sm_insite::after_enter(tdf_state_machine &_sm)
{
}
void vehicle_sm_insite::before_leave(tdf_state_machine &_sm)
{
}
std::string vehicle_sm_insite::state_name()
{
    return "场内";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_do_scale::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.weight_stable())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_after_scale());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_do_scale::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.record_weight();
    vsm.record_exit_gate_code();
}
void vehicle_sm_do_scale::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.start_scale_timer();
}
void vehicle_sm_do_scale::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.close_all_timer();
}
std::string vehicle_sm_do_scale::state_name()
{
    return "称重";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_before_scale::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.cannot_enter_scale())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_insite());
    }
    else if (vsm.can_enter_scale())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_enter_scale());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_before_scale::do_action(tdf_state_machine &_sm)
{
}
void vehicle_sm_before_scale::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.show_scale_info();
}
void vehicle_sm_before_scale::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.clean_show_info();
}
std::string vehicle_sm_before_scale::state_name()
{
    return "称重前";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_after_scale::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.vehicle_leave_scale())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_insite());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_after_scale::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.print_scale_info();
}
void vehicle_sm_after_scale::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.open_door();
}
void vehicle_sm_after_scale::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.close_door();
}
std::string vehicle_sm_after_scale::state_name()
{
    return "称重后";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_end::change_state(tdf_state_machine &_sm)
{
    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_end::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.vehicle_save();
    vsm.all_timer_handler.push_back(tdf_main::get_inst().start_timer(
        1,
        [](void *_private)
        {
            auto sm = (vehicle_state_machine *)_private;
            vehicle_state_machine::delete_sm(sm->plateNo);
        },
        &vsm));
}
void vehicle_sm_end::after_enter(tdf_state_machine &_sm)
{
}
void vehicle_sm_end::before_leave(tdf_state_machine &_sm)
{
}
std::string vehicle_sm_end::state_name()
{
    return "结束";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_enter_gate::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.vehicle_pass_gate())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_insite());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_enter_gate::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.record_enter_time();
}
void vehicle_sm_enter_gate::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.open_door();
    vsm.show_enter_info();
}
void vehicle_sm_enter_gate::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.close_door();
    vsm.clean_show_info();
}
std::string vehicle_sm_enter_gate::state_name()
{
    return "进厂";
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_before_exit_gate::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.can_leave())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_exit_gate());
    }
    else if (vsm.cannot_leave())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_insite());
    }

    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_before_exit_gate::do_action(tdf_state_machine &_sm)
{
}
void vehicle_sm_before_exit_gate::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.show_before_enter_info();
}
void vehicle_sm_before_exit_gate::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.clean_show_info();
}
std::string vehicle_sm_before_exit_gate::state_name()
{
    return "出门前";
}
bool vehicle_state_machine::vehicle_on_scale()
{
    tdf_state_machine_lock tmp_lock(*this);
    return related_gate_code.length() > 0 && leave_flag == true;
}
bool vehicle_state_machine::vehicle_pass_gate()
{
    tdf_state_machine_lock tmp_lock(*this);
    return related_gate_code.length() > 0 && leave_flag == true;
}
bool vehicle_state_machine::gate_exit_near()
{
    bool ret = false;
    tdf_state_machine_lock tmp_lock(*this);

    if (related_gate_code.length() > 0)
    {
        device_config tmp;
        system_management_handler::get_inst()->internal_get_device_config(tmp);
        for (auto &itr : tmp.gate)
        {
            if (itr.exit == related_gate_code)
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}
bool vehicle_state_machine::scale_entry_near()
{
    bool ret = false;

    tdf_state_machine_lock tmp_lock(*this);
    if (related_gate_code.length() > 0)
    {
        device_config tmp;
        system_management_handler::get_inst()->internal_get_device_config(tmp);
        for (auto &itr : tmp.scale)
        {
            if (itr.entry == related_gate_code || itr.exit == related_gate_code)
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}
bool vehicle_state_machine::weight_stable()
{
    bool ret = false;

    if (continue_weight.size() > 8)
    {
        ret = true;
    }

    return ret;
}
bool vehicle_state_machine::vehicle_leave_scale()
{
    tdf_state_machine_lock tmp_lock(*this);
    return related_gate_code.length() > 0 && leave_flag == true;
}
bool vehicle_state_machine::enter_timeout()
{
    return enter_timeout_flag;
}
void vehicle_state_machine::close_all_timer()
{
    for (auto &itr : all_timer_handler)
    {
        tdf_main::get_inst().stop_timer(itr);
    }
    all_timer_handler.clear();
}
void vehicle_state_machine::clean_show_info()
{
}
void vehicle_state_machine::record_cur_scale()
{
    tdf_state_machine_lock tmp_lock(*this);
    device_config tmp;
    system_management_handler::get_inst()->internal_get_device_config(tmp);
    for (auto &itr : tmp.scale)
    {
        if (itr.entry == related_gate_code || itr.exit == related_gate_code)
        {
            cur_scale = itr;
            cur_enter_scale_gate = related_gate_code;
            if (itr.entry == related_gate_code)
            {
                cur_exit_scale_gate = itr.exit;
            }
            else
            {
                cur_exit_scale_gate = itr.entry;
            }
            break;
        }
    }
}
void vehicle_state_machine::start_scale_timer()
{
    all_timer_handler.push_back(tdf_main::get_inst().start_timer(
        1,
        [](void *_private)
        {
            auto vsm = (vehicle_state_machine *)_private;
            if (!raster_was_block(vsm->cur_scale.raster_ip[0], ZH_RASTER_PORT) && !raster_was_block(vsm->cur_scale.raster_ip[1], ZH_RASTER_PORT))
            {
                vsm->continue_weight.push_back(50.1);
                vsm->trigger_sm();
            }
        },
        this));
}
void vehicle_state_machine::start_enter_timer()
{
    all_timer_handler.push_back(tdf_main::get_inst().start_timer(
        1,
        [](void *_private)
        {
            auto vsm = (vehicle_state_machine *)_private;
            device_config tmp;
            system_management_handler::get_inst()->internal_get_device_config(tmp);
            for (auto &itr : tmp.gate)
            {
                if (itr.entry == vsm->related_gate_code)
                {
                    auto id_ret = zh_read_id_no(itr.entry_id_reader_ip, ZH_ID_READER_PORT);
                    if (id_ret.length() > 0)
                    {
                        vsm->id_no = id_ret;
                        vsm->trigger_sm();
                    }
                    break;
                }
            }
        },
        this));
    all_timer_handler.push_back(tdf_main::get_inst().start_timer(
        60, [](void *_private)
        {
            auto vsm = (vehicle_state_machine *)_private;
            vsm->enter_timeout_flag = true;
            vsm->trigger_sm();
        },
        this));
}

void vehicle_state_machine::open_door()
{
    tdf_state_machine_lock tmp_lock(*this);
    zh_hk_ctrl_gate(related_gate_code, zh_hk_gate_control_cmd::zh_hk_gate_open);
    leave_flag = false;
}
void vehicle_state_machine::close_door()
{
    tdf_state_machine_lock tmp_lock(*this);
    zh_hk_ctrl_gate(related_gate_code, zh_hk_gate_control_cmd::zh_hk_gate_close);
    related_gate_code = "";
}

void vehicle_state_machine::show_exit_info()
{
    tdf_state_machine_lock tmp_lock(*this);
    zh_hk_ctrl_led(related_gate_code, "一路顺风");
    zh_hk_ctrl_voice(related_gate_code, "一路顺风");
}
void vehicle_state_machine::show_enter_info()
{
    tdf_state_machine_lock tmp_lock(*this);
    zh_hk_ctrl_led(related_gate_code, "请入厂");
    zh_hk_ctrl_voice(related_gate_code, "请入厂");
}
void vehicle_state_machine::print_scale_info()
{
    std::string content = plateNo + "\n";
    content += "一次称重：" + std::to_string(first_detail.weight) + "\n";
    content += "一次称重时间：" + first_detail.timestamp + "\n";
    if (second_detail.timestamp.length() > 0)
    {
        content += "二次称重：" + std::to_string(second_detail.weight) + "\n";
        content += "二次称重时间：" + second_detail.timestamp + "\n";
    }
    device_config tmp;

    std::string printer_ip;
    if (cur_scale.entry == cur_exit_scale_gate)
    {
        printer_ip = cur_scale.entry_printer_ip;
    }
    else if (cur_scale.exit == cur_exit_scale_gate)
    {
        printer_ip = cur_scale.exit_printer_ip;
    }

    std::string print_cmd = "/bin/zh_sprt_printer " + printer_ip + " \"" + content + "\"";

    auto fp = popen(print_cmd.c_str(), "r");
    if (fp)
    {
        tdf_log printer_log("printer");
        std::string err_ret;
        char tmp;
        while (fread(&tmp, 1, 1, fp) > 0)
        {
            err_ret.push_back(tmp);
        }
        printer_log.err(err_ret);
        pclose(fp);
    }
}

void vehicle_state_machine::proc_gate_near(const std::string &_code)
{
    tdf_state_machine_lock tmp_lock(*this);
    related_gate_code = _code;
    trigger_sm();
}
void vehicle_state_machine::proc_pass_gate(const std::string &_code)
{
    tdf_state_machine_lock tmp_lock(*this);
    if (related_gate_code == _code)
    {
        leave_flag = true;
        trigger_sm();
    }
}

void vehicle_state_machine::record_weight()
{
    if (first_detail.timestamp.length() <= 0)
    {
        first_detail.name = cur_scale.name;
        first_detail.timestamp = zh_rpc_util_get_timestring();
        first_detail.weight =
            [this]() -> double
        {
            double ret = 0;
            for (auto &itr : continue_weight)
            {
                ret += itr;
            }
            ret /= continue_weight.size();

            return ret;
        }();
    }
    else if (second_detail.timestamp.length() <= 0)
    {
        second_detail.name = cur_scale.name;
        second_detail.timestamp = zh_rpc_util_get_timestring();
        second_detail.weight =
            [this]() -> double
        {
            double ret = 0;
            for (auto &itr : continue_weight)
            {
                ret += itr;
            }
            ret /= continue_weight.size();

            return ret;
        }();
    }
}
void vehicle_state_machine::record_exit_gate_code()
{
    tdf_state_machine_lock tmp_lock(*this);
    related_gate_code = cur_exit_scale_gate;
}
void vehicle_state_machine::record_enter_time()
{
    tdf_state_machine_lock tmp_lock(*this);
    enter_detail.timestamp = zh_rpc_util_get_timestring();
    device_config tmp;
    system_management_handler::get_inst()->internal_get_device_config(tmp);
    for (auto &itr : tmp.gate)
    {
        if (itr.entry == related_gate_code)
        {
            enter_detail.name = itr.name;
            break;
        }
    }
}
void vehicle_state_machine::record_exit_time()
{
    tdf_state_machine_lock tmp_lock(*this);
    exit_detail.timestamp = zh_rpc_util_get_timestring();
    device_config tmp;
    system_management_handler::get_inst()->internal_get_device_config(tmp);
    for (auto &itr : tmp.gate)
    {
        if (itr.exit == related_gate_code)
        {
            exit_detail.name = itr.name;
            break;
        }
    }
}

void vehicle_state_machine::show_scale_info()
{
    zh_hk_ctrl_led(related_gate_code, "请稍后");
}

std::map<std::string, std::shared_ptr<vehicle_state_machine>> vehicle_state_machine::all_sm;

vehicle_state_machine &vehicle_state_machine::fetch_sm(const std::string &_plateNo, int _type)
{
    if (all_sm.find(_plateNo) != all_sm.end())
    {
        return *(all_sm[_plateNo]);
    }
    switch (_type)
    {
    case 1:
    {
        auto sm = std::make_shared<none_vehicle_sm>(_plateNo, _type);
        all_sm[_plateNo] = sm;
        return *sm;
        break;
    }

    default:
    {
        auto sm = std::make_shared<none_vehicle_sm>(_plateNo, _type);
        all_sm[_plateNo] = sm;
        return *sm;
        break;
    }
    }
}

void vehicle_state_machine::delete_sm(const std::string &_plateNo)
{
    if (all_sm.find(_plateNo) != all_sm.end())
    {
        if (all_sm[_plateNo]->m_cur_state->state_name() == "结束")
        {
            all_sm.erase(_plateNo);
        }
    }
}

bool none_vehicle_sm::can_enter_scale()
{
    return true;
}
bool none_vehicle_sm::cannot_enter_scale()
{
    return false;
}
bool none_vehicle_sm::can_leave()
{
    return true;
}
bool none_vehicle_sm::cannot_leave()
{
    return false;
}
bool none_vehicle_sm::can_enter()
{
    return true;
}
bool none_vehicle_sm::cannot_enter()
{
    return true;
}
void none_vehicle_sm::show_before_enter_info()
{
    tdf_state_machine_lock tmp_lock(*this);
    zh_hk_ctrl_led(related_gate_code, "可进");
    zh_hk_ctrl_voice(related_gate_code, "可进");
}
void none_vehicle_sm::show_before_exit_info()
{
    tdf_state_machine_lock tmp_lock(*this);
    zh_hk_ctrl_led(related_gate_code, "可出");
    zh_hk_ctrl_voice(related_gate_code, "可出");
}
void none_vehicle_sm::vehicle_save()
{
}