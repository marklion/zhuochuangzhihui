#include "vehicle_order_center_imp.h"
#include "zh_rpc_util.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"
#include "../zh_id_reader/lib/zh_id_reader.h"
#include "system_management_imp.h"
#include "../zh_raster/lib/zh_raster.h"
#include "../zh_scale/lib/zh_scale.h"

vehicle_order_center_handler *vehicle_order_center_handler::m_inst = nullptr;
std::map<std::string, std::shared_ptr<vehicle_state_machine>> vehicle_state_machine::all_sm;
std::map<std::string, std::shared_ptr<scale_state_machine>> vehicle_order_center_handler::ssm_map;
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

    auto orders = sqlite_orm::search_record_all<zh_sql_vehicle_order>("%s ORDER BY PRI_ID DESC LIMIT 30 OFFSET %ld", contract_query.c_str(), anchor);
    for (auto &itr : orders)
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
        for (auto &single_status : all_status)
        {
            order_status_info status_detail;
            status_detail.name = single_status.name;
            status_detail.step = single_status.step;
            status_detail.timestamp = single_status.timestamp;
            status_detail.user_name = single_status.user_name;
            tmp.status_details.push_back(status_detail);
        }
        tmp.p_weight = itr.p_weight;
        tmp.m_weight = itr.m_weight;

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
    for (auto &itr : order)
    {
        auto single_order = sqlite_orm::search_record<zh_sql_vehicle_order>(itr.id);
        if (!single_order || single_order->status == 2)
        {
            ZH_RETURN_ORDER_CANNOT_CANCEL(itr.main_vehicle_number);
        }
        need_cancel.push_back(*single_order);
    }

    for (auto &itr : need_cancel)
    {
        auto cancel_status = zh_sql_order_status::make_end_status(ssid);
        itr.push_status(cancel_status);
    }

    return ret;
}

std::shared_ptr<scale_state_machine> vehicle_order_center_handler::get_scale_sm(const std::string &_name)
{
    return ssm_map[_name];
}
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
        auto sm = std::make_shared<no_gate_vehicle_sm>(_plateNo, _type);
        all_sm[_plateNo] = sm;
        return *sm;
        break;
    }

    default:
    {
        auto sm = std::make_shared<no_gate_vehicle_sm>(_plateNo, _type);
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
bool vehicle_state_machine::scale_trigger()
{
    return scale_name.length() > 0;
}
bool vehicle_state_machine::enter_trigger()
{
    bool ret = false;
    device_config dc;
    system_management_handler::get_inst()->internal_get_device_config(dc);

    for (auto &itr : dc.gate)
    {
        if (itr.entry == current_roadway)
        {
            ret = true;
            break;
        }
    }

    return ret;
}
bool vehicle_state_machine::exit_trigger()
{
    bool ret = false;
    device_config dc;
    system_management_handler::get_inst()->internal_get_device_config(dc);

    for (auto &itr : dc.gate)
    {
        if (itr.exit == current_roadway)
        {
            ret = true;
            break;
        }
    }

    return ret;
}
void vehicle_state_machine::close_all_timer()
{
    for (auto &itr : all_timer_handler)
    {
        tdf_main::get_inst().stop_timer(itr);
    }
    all_timer_handler.clear();
    is_timeout = false;
}
bool vehicle_state_machine::timeout()
{
    return is_timeout;
}
bool vehicle_state_machine::scale_stable()
{
    return cur_weight != 0;
}
void vehicle_state_machine::close_id_read()
{
    tdf_main::get_inst().stop_timer(id_reader_timer_handle);
    id_reader_timer_handle = -1;
    id_no = "";
}
void vehicle_state_machine::clean_roadway()
{
    current_roadway = "";
    scale_name = "";
}
void vehicle_state_machine::open_door()
{
    zh_hk_ctrl_gate(current_roadway, zh_hk_gate_open);
}
void vehicle_state_machine::close_scale()
{
    cur_weight = 0;
}
void vehicle_state_machine::record_enter()
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", plateNo.c_str());
    if (vo)
    {
        auto status = zh_sql_order_status::make_in_status();
        vo->push_status(status);
    }
}
void vehicle_state_machine::record_exit()
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", plateNo.c_str());
    if (vo)
    {
        auto status = zh_sql_order_status::make_out_status();
        vo->push_status(status);
    }
}
void vehicle_state_machine::enable_id_read()
{
    id_reader_timer_handle = tdf_main::get_inst().start_timer(
        1,
        [](void *_private)
        {
            auto vsm = (vehicle_state_machine *)_private;
            device_config dc;
            system_management_handler::get_inst()->internal_get_device_config(dc);
            std::string id_reader_ip = "";
            for (auto &itr : dc.gate)
            {
                if (itr.entry == vsm->current_roadway)
                {
                    id_reader_ip = itr.entry_id_reader_ip;
                    break;
                }
            }
            auto read_ret = zh_read_id_no(id_reader_ip, ZH_ID_READER_PORT);
            if (read_ret.length() > 0)
            {
                vsm->id_no = read_ret;
                vsm->trigger_sm();
            }
        },
        this);
}
void vehicle_state_machine::open_enter_timer()
{
    all_timer_handler.push_back(tdf_main::get_inst().start_timer(
        60,
        [](void *_private)
        {
            auto vsm = (vehicle_state_machine *)_private;
            vsm->is_timeout = true;
            vsm->trigger_sm();
        },
        this));
}
void vehicle_state_machine::open_exit_timer()
{
    all_timer_handler.push_back(tdf_main::get_inst().start_timer(
        60,
        [](void *_private)
        {
            auto vsm = (vehicle_state_machine *)_private;
            vsm->is_timeout = true;
            vsm->trigger_sm();
        },
        this));
}

void vehicle_state_machine::proc_roadway_trigger(const std::string &_roadway)
{
    if (current_roadway.length() <= 0)
    {
        current_roadway = _roadway;
        trigger_sm();
    }
}

void vehicle_state_machine::enable_scale()
{
    cur_weight = 0;
}
void vehicle_state_machine::record_close()
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", plateNo.c_str());
    if (vo)
    {
        auto status = zh_sql_order_status::make_end_status();
        vo->push_status(status);
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
        if (stuff)
        {
            stuff->inventory += vo->p_weight - vo->m_weight;
            stuff->update_record();
        }
    }
}
bool no_gate_vehicle_sm::can_scale()
{
    bool ret = false;
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100 AND status != 0", plateNo.c_str());
    if (vo)
    {
        if (vo->status < 4)
        {
            ret = true;
        }
    }

    return ret;
}
bool no_gate_vehicle_sm::can_exit()
{
    return false;
}
bool no_gate_vehicle_sm::finish_scale()
{
    bool ret = false;
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status == 4", plateNo.c_str());
    if (vo)
    {
        ret = true;
    }

    return ret;
}
bool no_gate_vehicle_sm::no_need_gate()
{
    return true;
}
bool no_gate_vehicle_sm::can_enter()
{
    return false;
}
void no_gate_vehicle_sm::print_weight()
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100 AND status != 0", plateNo.c_str());
    auto ssm = vehicle_order_center_handler::get_inst()->get_scale_sm(scale_name);
    if (vo && ssm)
    {
        if (vo->status == 3)
        {
            std::string content = "一次称重：" + std::to_string(vo->p_weight);
            ssm->proc_print_weight(content);
        }
        else if (vo->status == 4)
        {
            std::string content = "一次称重：" + std::to_string(vo->p_weight) + "\n二次称重：" + std::to_string(vo->m_weight);
            ssm->proc_print_weight(content);
        }
    }
}
void no_gate_vehicle_sm::record_weight()
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100 AND status != 0", plateNo.c_str());
    if (vo)
    {
        if (vo->status < 3)
        {
            auto status = zh_sql_order_status::make_p_status();
            vo->p_weight = cur_weight;
            vo->push_status(status);
        }
        else if (vo->status < 4)
        {
            auto status = zh_sql_order_status::make_m_status();
            vo->m_weight = cur_weight;
            vo->push_status(status);
        }
    }
}
void no_gate_vehicle_sm::enter_scale_cast()
{
    std::string content = "不允许上磅";
    if (can_scale())
    {
        content = "请上磅";
    }
    auto ssm = vehicle_order_center_handler::get_inst()->get_scale_sm(scale_name);
    if (ssm)
    {
        ssm->proc_enter_scale_cast(content);
    }
}
void no_gate_vehicle_sm::enter_gate_cast()
{
    std::string content = "不允许进场";
    if (can_scale())
    {
        content = "请进场";
    }
    zh_hk_ctrl_led(current_roadway, content);
    zh_hk_ctrl_voice(current_roadway, content);
}
void no_gate_vehicle_sm::exit_gate_cast()
{
    std::string content = "不允许出场";
    if (can_scale())
    {
        content = "请出场";
    }
    zh_hk_ctrl_led(current_roadway, content);
    zh_hk_ctrl_voice(current_roadway, content);
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_before_enter_gate::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (!vsm.can_enter() || vsm.timeout())
    {
        vehicle_state_machine::delete_sm(vsm.plateNo);
    }
    else if (vsm.can_enter())
    {
        return std::unique_ptr<vehicle_sm_insite>(new vehicle_sm_insite());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_before_enter_gate::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.enter_gate_cast();
    if (vsm.can_enter())
    {
        vsm.open_door();
        vsm.record_enter();
    }
}
void vehicle_sm_before_enter_gate::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.enable_id_read();
    vsm.open_enter_timer();
}
void vehicle_sm_before_enter_gate::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.close_id_read();
    vsm.close_all_timer();
    vsm.clean_roadway();
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_before_scale::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.can_scale())
    {
        auto ssm = vehicle_order_center_handler::get_inst()->get_scale_sm(vsm.scale_name);
        if (ssm)
        {
            ssm->rfv = scale_state_machine::accept;
            ssm->trigger_sm();
        }
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_scale());
    }
    else if (!vsm.can_scale())
    {
        auto ssm = vehicle_order_center_handler::get_inst()->get_scale_sm(vsm.scale_name);
        if (ssm)
        {
            ssm->rfv = scale_state_machine::reject;
            ssm->trigger_sm();
        }
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_insite());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_before_scale::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.can_scale())
    {
        vsm.enter_scale_cast();
    }
}
void vehicle_sm_before_scale::after_enter(tdf_state_machine &_sm)
{
}
void vehicle_sm_before_scale::before_leave(tdf_state_machine &_sm)
{
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_before_exit_gate::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.can_exit())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_end());
    }
    else if (!vsm.can_exit() || vsm.timeout())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_insite());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_before_exit_gate::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.can_exit())
    {
        vsm.open_door();
        vsm.exit_gate_cast();
        vsm.record_exit();
    }
}
void vehicle_sm_before_exit_gate::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.enable_id_read();
    vsm.open_exit_timer();
}
void vehicle_sm_before_exit_gate::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.close_id_read();
    vsm.close_all_timer();
    vsm.clean_roadway();
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_insite::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.scale_trigger())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_before_scale());
    }
    else if (vsm.finish_scale() && vsm.no_need_gate())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_end());
    }
    else if (vsm.exit_trigger())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_before_exit_gate());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_insite::do_action(tdf_state_machine &_sm)
{
}
void vehicle_sm_insite::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.clean_roadway();
}
void vehicle_sm_insite::before_leave(tdf_state_machine &_sm)
{
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_scale::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.scale_stable())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_insite());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_scale::do_action(tdf_state_machine &_sm)
{
}
void vehicle_sm_scale::after_enter(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.enable_scale();
}
void vehicle_sm_scale::before_leave(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.record_weight();
    vsm.print_weight();
    vsm.close_scale();
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_end::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vehicle_state_machine::delete_sm(vsm.plateNo);
    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_end::do_action(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    vsm.record_close();
}
void vehicle_sm_end::after_enter(tdf_state_machine &_sm)
{
}
void vehicle_sm_end::before_leave(tdf_state_machine &_sm)
{
}
std::unique_ptr<tdf_state_machine_state> vehicle_sm_init::change_state(tdf_state_machine &_sm)
{
    auto &vsm = dynamic_cast<vehicle_state_machine &>(_sm);
    if (vsm.enter_trigger())
    {
        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_before_enter_gate());
    }
    else if (vsm.scale_trigger())
    {

        return std::unique_ptr<tdf_state_machine_state>(new vehicle_sm_before_scale());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void vehicle_sm_init::do_action(tdf_state_machine &_sm)
{
}
void vehicle_sm_init::after_enter(tdf_state_machine &_sm)
{
}
void vehicle_sm_init::before_leave(tdf_state_machine &_sm)
{
}

scale_state_machine::scale_state_machine(const device_scale_config &_config) : m_log(_config.name + " scale sm"), bound_scale(_config)
{
    auto id_read_call_back_entry = [](void *_private)
    {
        auto ssm = (scale_state_machine *)_private;
        auto id_ret = zh_read_id_no(ssm->bound_scale.entry_id_reader_ip, ZH_ID_READER_PORT);
        if (id_ret.length() > 0)
        {
            auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("driver_id == '%s' AND status != 100 AND status != 0", id_ret.c_str());
            if (vo)
            {
                scale_trigger_event tmp;
                tmp.road_way = ssm->bound_scale.entry;
                tmp.vehicle_number = vo->main_vehicle_number;
                tdf_state_machine_lock a(*ssm);
                ssm->trigger_events.push_back(tmp);
            }
            ssm->trigger_sm();
        }
    };
    auto id_read_call_back_exit = [](void *_private)
    {
        auto ssm = (scale_state_machine *)_private;
        auto id_ret = zh_read_id_no(ssm->bound_scale.exit_id_reader_ip, ZH_ID_READER_PORT);
        if (id_ret.length() > 0)
        {
            auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("driver_id == '%s' AND status != 100 AND status != 0", id_ret.c_str());
            if (vo)
            {
                scale_trigger_event tmp;
                tmp.road_way = ssm->bound_scale.exit;
                tmp.vehicle_number = vo->main_vehicle_number;
                tdf_state_machine_lock a(*ssm);
                ssm->trigger_events.push_back(tmp);
            }
            ssm->trigger_sm();
        }
    };
    if (_config.entry_id_reader_ip.length() > 0)
    {
        id_read_timer.push_back(tdf_main::get_inst().start_timer(1, id_read_call_back_entry, this));
    }
    if (_config.exit_id_reader_ip.length() > 0)
    {
        id_read_timer.push_back(tdf_main::get_inst().start_timer(1, id_read_call_back_exit, this));
    }
    m_cur_state.reset(new scale_sm_idle());
}
scale_state_machine::~scale_state_machine()
{
    for (auto &itr : id_read_timer)
    {
        tdf_main::get_inst().stop_timer(itr);
    }
    id_read_timer.clear();
}
void scale_state_machine::convert_event()
{
    if (trigger_events.size() > 0)
    {
        auto new_event = trigger_events.front();
        trigger_events.pop_front();
        enter_roadway = new_event.road_way;
        if (enter_roadway == bound_scale.entry)
        {
            exit_roadway = bound_scale.exit;
        }
        else if (enter_roadway == bound_scale.exit)
        {
            exit_roadway = bound_scale.entry;
        }
        bound_vehicle_number = new_event.vehicle_number;
    }
}
void scale_state_machine::trigger_vehicle_sm_vehicle_come()
{
    if (sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100 AND status != 0", bound_vehicle_number.c_str()))
    {
        auto &vsm = vehicle_state_machine::fetch_sm(bound_vehicle_number, 1);
        vsm.scale_name = bound_scale.name;
        vsm.trigger_sm();
    }
}
void scale_state_machine::trigger_vehicle_sm_scale_finish()
{
    if (sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100 AND status != 0", bound_vehicle_number.c_str()))
    {
        auto &vsm = vehicle_state_machine::fetch_sm(bound_vehicle_number, 1);
        vsm.cur_weight = [=]() -> double
        {
            double scale_ret = 0;
            for (auto &itr : continue_weight)
            {
                scale_ret += itr;
            }
            scale_ret /= continue_weight.size();
            return scale_ret;
        }();
        vsm.trigger_sm();
    }
}
void scale_state_machine::open_enter()
{
    zh_hk_ctrl_gate(enter_roadway, zh_hk_gate_open);
}
void scale_state_machine::open_exit()
{
    zh_hk_ctrl_gate(exit_roadway, zh_hk_gate_open);
}
void scale_state_machine::scale_zero()
{
    clean_scale_weight(bound_scale.scale_ip, ZH_SCALE_PORT);
}
void scale_state_machine::open_scale_timer()
{
    timer_fd = tdf_main::get_inst().start_timer(
        1,
        [](void *_private)
        {
            auto ssm = (scale_state_machine *)_private;
            if (!raster_was_block(ssm->bound_scale.raster_ip[0], ZH_RASTER_PORT) && !raster_was_block(ssm->bound_scale.raster_ip[1], ZH_RASTER_PORT))
            {
                auto scale_ret = get_current_weight(ssm->bound_scale.scale_ip, ZH_SCALE_PORT);
                ssm->continue_weight.push_back(scale_ret);
                auto ava = [=]() -> double
                {
                    double sum = 0;
                    for (auto &itr : ssm->continue_weight)
                    {
                        sum += itr;
                    }
                    return sum / ssm->continue_weight.size();
                }();
                double p_dev;
                for (auto &itr : ssm->continue_weight)
                {
                    p_dev += (ava - itr) * (ava - itr);
                }
                p_dev /= ssm->continue_weight.size();
                p_dev = sqrt(p_dev);
                if (p_dev / ava > 0.01)
                {
                    ssm->continue_weight.clear();
                }
                else
                {
                    ssm->trigger_sm();
                }
            }
            else
            {
                ssm->continue_weight.clear();
            }
        },
        this);
}
void scale_state_machine::close_timer()
{
    if (timer_fd >= 0)
    {
        tdf_main::get_inst().stop_timer(timer_fd);
        timer_fd = -1;
        continue_weight.clear();
    }
}
void scale_state_machine::clean_bound_info()
{
    enter_roadway = "";
    exit_roadway = "";
    auto itr = trigger_events.begin();
    for (; itr != trigger_events.end(); )
    {
        if (itr->vehicle_number == bound_vehicle_number)
        {
            itr = trigger_events.erase(itr);
        }
        else
        {
            itr++;
        }
    }
    bound_vehicle_number = "";
}
bool scale_state_machine::is_vehicle_come()
{
    if (bound_vehicle_number.length() > 0)
    {
        return true;
    }
    return false;
}
bool scale_state_machine::should_open()
{
    return rfv == accept;
}
bool scale_state_machine::should_not_open()
{
    return rfv == reject;
}
bool scale_state_machine::scale_stable()
{
    bool ret = false;
    if (continue_weight.size() > 8)
    {
        auto cur_weight = [=]() -> double
        {
            double scale_ret = 0;
            for (auto &itr : continue_weight)
            {
                scale_ret += itr;
            }
            scale_ret /= continue_weight.size();
            return scale_ret;
        }();
        if (cur_weight > 1)
        {
            ret = true;
        }
    }

    return ret;
}
bool scale_state_machine::scale_clear()
{
    bool ret = false;
    if (continue_weight.size() > 8)
    {
        auto cur_weight = [=]() -> double
        {
            double scale_ret = 0;
            for (auto &itr : continue_weight)
            {
                scale_ret += itr;
            }
            scale_ret /= continue_weight.size();
            return scale_ret;
        }();
        if (cur_weight < 1)
        {
            ret = true;
        }
    }

    return ret;
}

void scale_state_machine::proc_enter_scale_cast(const std::string &_content)
{
    zh_hk_ctrl_led(enter_roadway, _content);
    zh_hk_ctrl_voice(enter_roadway, _content);
}
void scale_state_machine::proc_finish_weight_cast(const std::string &_content)
{
    zh_hk_ctrl_led(exit_roadway, _content);
    zh_hk_ctrl_voice(exit_roadway, _content);
}
void scale_state_machine::proc_print_weight(const std::string &_content)
{
    if (exit_roadway == bound_scale.exit_printer_ip)
    {
        system_management_handler::get_inst()->print_content(bound_scale.exit_printer_ip, _content);
    }
    else if (exit_roadway == bound_scale.entry_printer_ip)
    {
        system_management_handler::get_inst()->print_content(bound_scale.entry_printer_ip, _content);
    }
}

std::unique_ptr<tdf_state_machine_state> scale_sm_idle::change_state(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    if (ssm.is_vehicle_come())
    {
        return std::unique_ptr<tdf_state_machine_state>(new scale_sm_vehicle_come());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void scale_sm_idle::do_action(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.convert_event();
}
void scale_sm_idle::after_enter(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.clean_bound_info();
}
void scale_sm_idle::before_leave(tdf_state_machine &_sm)
{
}
std::unique_ptr<tdf_state_machine_state> scale_sm_vehicle_come::change_state(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    if (ssm.should_open())
    {
        return std::unique_ptr<tdf_state_machine_state>(new scale_sm_scale());
    }
    else if (ssm.should_not_open())
    {
        return std::unique_ptr<tdf_state_machine_state>(new scale_sm_idle());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void scale_sm_vehicle_come::do_action(tdf_state_machine &_sm)
{
}
void scale_sm_vehicle_come::after_enter(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.trigger_vehicle_sm_vehicle_come();
}
void scale_sm_vehicle_come::before_leave(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.rfv = scale_state_machine::none;
}
std::unique_ptr<tdf_state_machine_state> scale_sm_scale::change_state(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    if (ssm.scale_stable())
    {
        return std::unique_ptr<tdf_state_machine_state>(new scale_sm_clean());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void scale_sm_scale::do_action(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    if (ssm.scale_stable())
    {
        ssm.trigger_vehicle_sm_scale_finish();
    }
}
void scale_sm_scale::after_enter(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.open_enter();
    ssm.scale_zero();
    ssm.open_scale_timer();
}
void scale_sm_scale::before_leave(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.close_timer();
    ssm.open_exit();
}
std::unique_ptr<tdf_state_machine_state> scale_sm_clean::change_state(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    if (ssm.scale_clear())
    {
        return std::unique_ptr<tdf_state_machine_state>(new scale_sm_idle());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void scale_sm_clean::do_action(tdf_state_machine &_sm)
{
}
void scale_sm_clean::after_enter(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.open_scale_timer();
}
void scale_sm_clean::before_leave(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.close_timer();
    ssm.clean_bound_info();
}