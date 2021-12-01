#include "vehicle_order_center_imp.h"
#include "zh_rpc_util.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"
#include "../zh_id_reader/lib/zh_id_reader.h"
#include "system_management_imp.h"
#include "../zh_raster/lib/zh_raster.h"
#include "../zh_scale/lib/zh_scale.h"

vehicle_order_center_handler *vehicle_order_center_handler::m_inst = nullptr;
std::map<std::string, std::shared_ptr<scale_state_machine>> vehicle_order_center_handler::ssm_map;
std::map<std::string, std::shared_ptr<gate_state_machine>> vehicle_order_center_handler::gsm_map;
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

scale_state_machine::scale_state_machine(const device_scale_config &_config) : m_log(_config.name + " scale sm"), bound_scale(_config)
{
    auto id_read_call_back_entry = [](void *_private)
    {
        auto ssm = (scale_state_machine *)_private;
        auto id_ret = zh_read_id_no(ssm->bound_scale.entry_id_reader_ip, ZH_ID_READER_PORT);
        if (id_ret.length() > 0)
        {
            ssm->proc_trigger_id_read(id_ret, ssm->bound_scale.entry_id_reader_ip);
            ssm->trigger_sm();
        }
    };
    auto id_read_call_back_exit = [](void *_private)
    {
        auto ssm = (scale_state_machine *)_private;
        auto id_ret = zh_read_id_no(ssm->bound_scale.exit_id_reader_ip, ZH_ID_READER_PORT);
        if (id_ret.length() > 0)
        {
            ssm->proc_trigger_id_read(id_ret, ssm->bound_scale.exit_id_reader_ip);
            ssm->trigger_sm();
        }
    };
    auto hk_call_back = [](const std::string &_plate_no, const std::string &_road_ip, void *_pdata)
    {
        if (_plate_no.length() > 0)
        {
            auto pthis = (scale_state_machine *)_pdata;
            pthis->proc_trigger_vehicle(_plate_no, _road_ip);
            pthis->trigger_sm();
        }
    };
    hk_sub_callback_cfg tmp_cfg;
    tmp_cfg.pData = this;
    tmp_cfg.callback = hk_call_back;
    if (_config.entry_id_reader_ip.length() > 0)
    {
        id_read_timer.push_back(tdf_main::get_inst().start_timer(1, id_read_call_back_entry, this));
    }
    if (_config.exit_id_reader_ip.length() > 0)
    {
        id_read_timer.push_back(tdf_main::get_inst().start_timer(1, id_read_call_back_exit, this));
    }
    if (_config.entry_config.cam_ip.length() > 0)
    {
        zh_hk_subcribe_event(_config.entry_config.cam_ip, tmp_cfg);
    }
    if (_config.exit_config.cam_ip.length() > 0)
    {
        zh_hk_subcribe_event(_config.exit_config.cam_ip, tmp_cfg);
    }
    m_cur_state.reset(new scale_sm_vehicle_come());
}
scale_state_machine::~scale_state_machine()
{
    for (auto &itr : id_read_timer)
    {
        tdf_main::get_inst().stop_timer(itr);
    }
    id_read_timer.clear();
    close_timer();
    zh_hk_unsubcribe_event(bound_scale.entry_config.cam_ip);
    zh_hk_unsubcribe_event(bound_scale.exit_config.cam_ip);
}

void scale_state_machine::open_enter()
{
    zh_hk_ctrl_gate(enter_cam_ip, zh_hk_gate_open);
}
void scale_state_machine::open_exit()
{
    zh_hk_ctrl_gate(exit_cam_ip, zh_hk_gate_open);
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
    enter_cam_ip = "";
    exit_cam_ip = "";
    entry_param.clear();
    exit_param.clear();
    bound_vehicle_number = "";
}

bool scale_state_machine::should_open()
{
    bool ret = false;

    std::string tmp_vehicle_entry = ctrl_policy.pass_permit(entry_param.vehicle_number, entry_param.id_no, "");
    std::string tmp_vehicle_exit = ctrl_policy.pass_permit(exit_param.vehicle_number, exit_param.id_no, "");
    if (tmp_vehicle_entry.length() > 0 || tmp_vehicle_exit.length() > 0)
    {
        ret = true;
    }

    return ret;
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

void scale_state_machine::open_trigger_switch()
{
    trigger_switch = true;
    zh_hk_manual_trigger(enter_cam_ip);
    zh_hk_manual_trigger(exit_cam_ip);
}
#define SCALE_BUSY_MSG_CONTENT "正在称重，请稍候"
void scale_state_machine::proc_trigger_id_read(const std::string &_id_no, const std::string &_id_reader_ip)
{
    tdf_state_machine_lock a(*this);
    if (_id_reader_ip == bound_scale.entry_id_reader_ip)
    {
        if (trigger_switch)
        {
            entry_param.id_no = _id_no;
        }
        else
        {
            zh_hk_ctrl_voice(bound_scale.entry_config.led_ip, SCALE_BUSY_MSG_CONTENT);
        }
    }
    if (_id_reader_ip == bound_scale.exit_id_reader_ip)
    {
        if (trigger_switch)
        {
            exit_param.id_no = _id_no;
        }
        else
        {
            zh_hk_ctrl_voice(bound_scale.exit_config.led_ip, SCALE_BUSY_MSG_CONTENT);
        }
    }
}
void scale_state_machine::proc_trigger_vehicle(const std::string &_vehicle_number, const std::string &_road_ip)
{
    tdf_state_machine_lock a(*this);
    if (_road_ip == bound_scale.entry_config.cam_ip)
    {
        if (trigger_switch)
        {
            entry_param.vehicle_number = _vehicle_number;
        }
        else
        {
            zh_hk_ctrl_voice(bound_scale.entry_config.led_ip, SCALE_BUSY_MSG_CONTENT);
        }
    }
    if (_road_ip == bound_scale.exit_config.cam_ip)
    {
        if (trigger_switch)
        {
            exit_param.vehicle_number = _vehicle_number;
        }
        else
        {
            zh_hk_ctrl_voice(bound_scale.exit_config.led_ip, SCALE_BUSY_MSG_CONTENT);
        }
    }
}

void scale_state_machine::record_entry_exit()
{
    std::string tmp_vehicle = ctrl_policy.pass_permit(entry_param.vehicle_number, entry_param.id_no, "");
    if (!tmp_vehicle.empty())
    {
        bound_vehicle_number = tmp_vehicle;
        enter_cam_ip = bound_scale.entry_config.cam_ip;
        exit_cam_ip = bound_scale.exit_config.cam_ip;
        return;
    }

    tmp_vehicle = ctrl_policy.pass_permit(exit_param.vehicle_number, exit_param.id_no, "");
    if (!tmp_vehicle.empty())
    {
        bound_vehicle_number = tmp_vehicle;
        enter_cam_ip = bound_scale.exit_config.cam_ip;
        exit_cam_ip = bound_scale.entry_config.cam_ip;
        return;
    }
}

void scale_state_machine::broadcast_enter_scale()
{
    std::string content = bound_vehicle_number + "\n";
    content += "请上磅\n";
    std::string led_ip;
    if (bound_scale.entry_config.cam_ip == enter_cam_ip)
    {
        led_ip = bound_scale.entry_config.led_ip;
    }
    else if (bound_scale.exit_config.cam_ip == enter_cam_ip)
    {
        led_ip = bound_scale.exit_config.led_ip;
    }
    if (led_ip.length() > 0)
    {
        zh_hk_ctrl_led(led_ip, content);
        zh_hk_ctrl_voice(led_ip, content);
    }
}
void scale_state_machine::broadcast_leave_scale()
{
    std::string content = bound_vehicle_number + "\n";
    content += "请下磅并取称重小票\n";
    std::string led_ip;
    if (bound_scale.entry_config.cam_ip == exit_cam_ip)
    {
        led_ip = bound_scale.entry_config.led_ip;
    }
    else if (bound_scale.exit_config.cam_ip == exit_cam_ip)
    {
        led_ip = bound_scale.exit_config.led_ip;
    }
    if (led_ip.length() > 0)
    {
        zh_hk_ctrl_led(led_ip, content);
        zh_hk_ctrl_voice(led_ip, content);
    }
}

std::unique_ptr<tdf_state_machine_state> scale_sm_vehicle_come::change_state(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    if (ssm.should_open())
    {
        return std::unique_ptr<tdf_state_machine_state>(new scale_sm_scale());
    }
    return std::unique_ptr<tdf_state_machine_state>();
}
void scale_sm_vehicle_come::do_action(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    if (ssm.should_open())
    {
        ssm.record_entry_exit();
        ssm.open_enter();
    }
}
void scale_sm_vehicle_come::after_enter(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.clean_bound_info();
    ssm.open_trigger_switch();
}
void scale_sm_vehicle_come::before_leave(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.trigger_switch = false;
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
        auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", ssm.bound_vehicle_number.c_str());
        if (vo)
        {
            auto cur_weight = [&]() -> double
            {
                double ret = 0;
                for (auto &itr : ssm.continue_weight)
                {
                    ret += itr;
                }
                ret /= ssm.continue_weight.size();

                return ret;
            }();
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
}
void scale_sm_scale::after_enter(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.broadcast_enter_scale();
    ssm.scale_zero();
    ssm.open_scale_timer();
}
void scale_sm_scale::before_leave(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.close_timer();
    ssm.open_exit();
    ssm.broadcast_leave_scale();
}
std::unique_ptr<tdf_state_machine_state> scale_sm_clean::change_state(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    if (ssm.scale_clear())
    {
        return std::unique_ptr<tdf_state_machine_state>(new scale_sm_vehicle_come());
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
}
gate_state_machine::gate_state_machine(const std::string &_road_ip, const std::string &_id_reader_ip, bool _is_entry) : m_log("gate sm"), road_ip(_road_ip), id_reader_ip(_id_reader_ip), is_entry(_is_entry)
{
    if (_id_reader_ip.length() > 0)
    {
        id_reader_timer = tdf_main::get_inst().start_timer(
            1,
            [](void *_private)
            {
                auto gsm = (gate_state_machine *)_private;
                auto id_no = zh_read_id_no(gsm->road_ip, ZH_ID_READER_PORT);
                if (id_no.length() > 0)
                {
                    gsm->proc_trigger_id_no(id_no);
                    gsm->trigger_sm();
                }
            },
            this);
    }
    if (_road_ip.length() > 0)
    {
        hk_sub_callback_cfg tmp_cfg;
        tmp_cfg.pData = this;
        tmp_cfg.callback = [](const std::string &_plate_no, const std::string &_road_ip, void *_pdata)
        {
            if (_plate_no.length() > 0)
            {
                auto pthis = (gate_state_machine *)_pdata;
                pthis->proc_trigger_vehicle_number(_plate_no);
                pthis->trigger_sm();
            }
        };
        zh_hk_subcribe_event(_road_ip, tmp_cfg);
    }
    m_cur_state.reset(new gate_sm_vehicle_come());
}
gate_state_machine::~gate_state_machine()
{
    if (id_reader_timer >= 0)
    {
        tdf_main::get_inst().stop_timer(id_reader_timer);
        id_reader_timer = -1;
    }
    zh_hk_unsubcribe_event(road_ip);
}
void gate_state_machine::clean_bound_info()
{
    param.clear();
}
void gate_state_machine::open_door()
{
    zh_hk_ctrl_gate(road_ip, zh_hk_gate_open);
}
void gate_state_machine::gate_cast_accept()
{
    std::string content = param.vehicle_number + "\n";
    if (is_entry)
    {
        content += "请进厂\n";
    }
    else
    {
        content += "一路顺风\n";
    }
    zh_hk_ctrl_led(road_ip, content);
    zh_hk_ctrl_voice(road_ip, content);
}
void gate_state_machine::gate_cast_reject()
{
    std::string content = param.vehicle_number + "\n";
    if (is_entry)
    {
        content += "不允许进厂\n";
    }
    else
    {
        content += "不允许出厂\n";
    }
    zh_hk_ctrl_led(road_ip, content);
    zh_hk_ctrl_voice(road_ip, content);
}
bool gate_state_machine::should_open()
{
    if (ctrl_policy.pass_permit(param.vehicle_number, param.id_no, "").length() > 0)
    {
        return true;
    }
    return false;
}

void gate_state_machine::record_vehicle_pass()
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", ctrl_policy.pass_permit(param.vehicle_number, param.id_no, "").c_str());
    if (vo && vo->status == 1)
    {
        auto status = zh_sql_order_status::make_in_status();
        vo->push_status(status);
    }
}

void gate_state_machine::proc_trigger_id_no(const std::string &_id_no)
{
    param.id_no = _id_no;
}
void gate_state_machine::proc_trigger_vehicle_number(const std::string &_vehicle_number)
{
    param.vehicle_number = _vehicle_number;
}
std::shared_ptr<gate_state_machine> vehicle_order_center_handler::get_gate_sm(const std::string &_road_way)
{
    return gsm_map[_road_way];
}

std::unique_ptr<tdf_state_machine_state> gate_sm_vehicle_come::change_state(tdf_state_machine &_sm)
{
    return std::unique_ptr<tdf_state_machine_state>();
}
void gate_sm_vehicle_come::do_action(tdf_state_machine &_sm)
{
    auto &gsm = dynamic_cast<gate_state_machine &>(_sm);
    if (gsm.should_open())
    {
        gsm.open_door();
        gsm.gate_cast_accept();
        gsm.record_vehicle_pass();
        gsm.clean_bound_info();
    }
    else
    {
        gsm.gate_cast_reject();
    }
}
void gate_sm_vehicle_come::after_enter(tdf_state_machine &_sm)
{
}
void gate_sm_vehicle_come::before_leave(tdf_state_machine &_sm)
{
}

std::string gate_ctrl_policy::pass_permit(const std::string &_vehicle_number, const std::string &_id_no, const std::string &_qr_code)
{
    std::string ret;
    bool judge_permit = true;

    if (need_id && _id_no.length() <= 0)
    {
        judge_permit = false;
    }
    if (need_qr && _qr_code.length() <= 0)
    {
        judge_permit = false;
    }

    if (judge_permit)
    {
        auto vos = sqlite_orm::search_record_all<zh_sql_vehicle_order>("(driver_id = '%s' OR order_number == '%s' OR main_vehicle_number = '%s') AND status != 100", _id_no.c_str(), _qr_code.c_str(), _vehicle_number.c_str());
        for (auto &itr : vos)
        {
            if (need_id && itr.driver_id != _id_no)
            {
                continue;
            }
            if (need_qr && itr.order_number != _qr_code)
            {
                continue;
            }
            ret = itr.main_vehicle_number;
            break;
        }
    }

    return ret;
}