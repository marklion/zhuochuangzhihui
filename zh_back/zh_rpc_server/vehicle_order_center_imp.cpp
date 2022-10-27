#include "vehicle_order_center_imp.h"
#include "zh_rpc_util.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"
#include "../zh_id_reader/lib/zh_id_reader.h"
#include "system_management_imp.h"
#include "contract_management_imp.h"
#include "../zh_raster/lib/zh_raster.h"
#include "../zh_scale/lib/zh_scale.h"
#include "../zh_qr/lib/zh_qr_lib.h"
#include "plugin_management_imp.h"
#include "../zh_traffic_light/lib/zh_traffic_light.h"
#include <sstream>

vehicle_order_center_handler *vehicle_order_center_handler::m_inst = nullptr;
std::map<std::string, std::shared_ptr<scale_state_machine>> vehicle_order_center_handler::ssm_map;
std::map<std::string, std::shared_ptr<gate_state_machine>> vehicle_order_center_handler::gsm_map;

static void generate_order_event(zh_sql_vehicle_order &_order)
{
    plugin_event_info pei;
    pei.order_number = _order.order_number;
    pei.type = (plugin_event_info::event_type)(_order.status);
    auto pmh = plugin_management_handler::get_inst();
    if (pmh)
    {
        pmh->deliver_event(pei);
    }
}

static bool change_order_status(zh_sql_vehicle_order &_order,zh_sql_order_status &_status, const zh_order_save_hook &_hook = zh_order_save_hook())
{
    _order.push_status(_status, _hook);
    generate_order_event(_order);
    return true;
}

static bool plugin_is_installed(const std::string &_plugin_name)
{
    bool ret = false;

    auto pmh = plugin_management_handler::get_inst();
    if (pmh)
    {
        auto installed_plugin = pmh->internel_get_installed_plugins();
        if (std::find_if(installed_plugin.begin(), installed_plugin.end(), [&](const std::string &_item)
                         { return _item == _plugin_name; }) != installed_plugin.end())
        {
            std::string std_out;
            std::string std_err;
            pmh->zh_plugin_run_plugin("enabled", _plugin_name, std_out, std_err);
            if (std_err.empty())
            {
                ret = true;
            }
        }
    }

    return ret;
}
static bool balance_enough(zh_sql_contract &_company, zh_sql_stuff &_stuff, int _vehicle_count, double *_already_cost, bool _only_self = false)
{
    bool ret = false;

    if (!_company.is_sale)
    {
        ret = true;
    }
    else if (_stuff.price <= 0 || _stuff.expect_weight <= 0)
    {
        ret = true;
    }
    else
    {
        std::string status_only_confirm = " AND status != 0 ";
        if (!_only_self)
        {
            status_only_confirm = "";
        }
        auto exist_order = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status != 100 %s AND company_name == '%s'", status_only_confirm.c_str(), _company.name.c_str());
        double already_cost = 0;
        for (auto &itr : exist_order)
        {
            auto single_stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", itr.stuff_name.c_str());
            if (single_stuff)
            {
                already_cost += single_stuff->price * single_stuff->expect_weight;
            }
        }
        if (_already_cost)
            *_already_cost = already_cost;
        auto single_price = _stuff.price;
        auto bound_price = sqlite_orm::search_record<zh_sql_contract_stuff_price>("customer_name == '%s' AND stuff_name == '%s'", _company.name.c_str(), _stuff.name.c_str());
        if (bound_price)
        {
            single_price = bound_price->price;
        }
        auto total_cost = already_cost + _vehicle_count * _stuff.expect_weight * single_price;
        if ((_company.balance + _company.credit) >= total_cost)
        {
            ret = true;
        }
    }

    return ret;
}
static void make_vehicle_detail_from_sql(vehicle_order_detail &_return, zh_sql_vehicle_order &_order)
{
    auto vo = &_order;
    vehicle_order_info tmp;
    tmp.behind_vehicle_number = vo->behind_vehicle_number;
    tmp.driver_id = vo->driver_id;
    tmp.driver_name = vo->driver_name;
    tmp.driver_phone = vo->driver_phone;
    tmp.id = vo->get_pri_id();
    tmp.main_vehicle_number = vo->main_vehicle_number;
    tmp.order_number = vo->order_number;
    tmp.status = vo->status;
    tmp.stuff_name = vo->stuff_name;
    tmp.company_name = vo->company_name;
    auto all_status = vo->get_all_children<zh_sql_order_status>("belong_order");
    for (auto &single_status : all_status)
    {
        order_status_info status_detail;
        status_detail.name = single_status.name;
        status_detail.step = single_status.step;
        status_detail.timestamp = single_status.timestamp;
        status_detail.user_name = single_status.user_name;
        tmp.status_details.push_back(status_detail);
    }
    tmp.p_weight = vo->p_weight;
    tmp.m_weight = vo->m_weight;
    auto attachment = vo->get_parent<zh_sql_file>("attachment");
    if (attachment)
    {
        tmp.attachment = attachment->name;
    }
    auto enter_weight_attachment = vo->get_parent<zh_sql_file>("enter_weight_attachment");
    if (enter_weight_attachment)
    {
        tmp.enter_weight_attachment = enter_weight_attachment->name;
    }
    tmp.enter_weight = vo->enter_weight;
    tmp.need_enter_weight = vo->need_enter_weight;
    tmp.company_address = vo->company_address;
    tmp.use_for = vo->use_for;
    tmp.max_count = vo->max_count;
    auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", tmp.company_name.c_str());
    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", tmp.stuff_name.c_str());
    if (company && stuff && company->is_sale && tmp.status == 0)
    {
        double already_cost = 0;
        if (!balance_enough(*company, *stuff, 1, &already_cost, true))
        {
            tmp.balance_warn = "余额不足，还差" + zh_double2string_reserve2(already_cost + stuff->expect_weight * stuff->price - company->balance - company->credit) + "元";
        }
    }
    _return.basic_info = tmp;
    _return.confirmed = vo->m_permit;
    _return.has_called = vo->m_called;
    _return.registered = vo->m_registered;
    _return.enter_nvr_ip = vo->enter_nvr_ip;
    _return.exit_nvr_ip = vo->exit_nvr_ip;
    _return.p_nvr_ip1 = vo->p_nvr_ip1;
    _return.p_nvr_ip2 = vo->p_nvr_ip2;
    _return.m_nvr_ip1 = vo->m_nvr_ip1;
    _return.m_nvr_ip2 = vo->m_nvr_ip2;
    _return.enter_time = vo->enter_cam_time;
    _return.exit_time = vo->exit_cam_time;
    _return.p_time = vo->p_cam_time;
    _return.m_time = vo->m_cam_time;
    _return.basic_info.end_time = vo->end_time;
    _return.basic_info.source_dest_name = vo->source_dest_name;
    _return.basic_info.price = vo->price;
    _return.checkin_time = vo->check_in_timestamp > 0 ? zh_rpc_util_get_timestring(vo->check_in_timestamp) : "";
    _return.call_time = vo->call_timestamp > 0 ? zh_rpc_util_get_timestring(vo->call_timestamp) : "";
    _return.basic_info.bl_number = vo->bl_number;
    _return.err_string = vo->err_string;
    _return.call_user_name = vo->call_user_name;
    if (company && company->is_sale)
    {
        _return.basic_info.is_sale = true;
    }
    else
    {
        _return.basic_info.is_sale = false;
    }
}
void vehicle_order_center_handler::get_order_by_anchor(std::vector<vehicle_order_info> &_return, const std::string &ssid, const int64_t anchor, const std::string &status_name, const std::string &enter_date)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    std::string detail_query = "PRI_ID != 0";
    auto contract = opt_user->get_parent<zh_sql_contract>("belong_contract");
    if (contract)
    {
        detail_query = "company_name == '" + contract->name + "'";
    }
    if (status_name.length() > 0)
    {
        long status = -1;
        if (status_name == "need_confirm")
        {
            status = 0;
        }
        else if (status_name == "has_not_came")
        {
            status = 1;
        }
        else if (status_name == "end")
        {
            status = 100;
        }
        if (status > -1)
        {
            detail_query += " AND status == " + std::to_string(status);
        }
        else if (status_name == "insite")
        {
            detail_query += " AND status > 1 AND status < 100";
        }
    }

    if (enter_date.length() > 0)
    {
        detail_query += " AND (PRI_ID == 0";
        auto date_status = sqlite_orm::search_record_all<zh_sql_order_status>("(step == 2 OR step == 3) AND timestamp LIKE '%s%%'", enter_date.c_str());
        for (auto &singel_date : date_status)
        {
            auto order = singel_date.get_parent<zh_sql_vehicle_order>("belong_order");
            if (order)
            {
                detail_query += " OR PRI_ID == " + std::to_string(order->get_pri_id());
            }
        }
        detail_query += ")";
    }

    auto orders = sqlite_orm::search_record_all<zh_sql_vehicle_order>("(%s) ORDER BY PRI_ID DESC LIMIT 30 OFFSET %ld", detail_query.c_str(), anchor);
    for (auto &itr : orders)
    {
        vehicle_order_detail tmp;
        make_vehicle_detail_from_sql(tmp, itr);
        _return.push_back(tmp.basic_info);
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

    auto exist_record = sqlite_orm::search_record<zh_sql_vehicle_order>("PRI_ID != %ld AND status != 100 AND (main_vehicle_number == '%s' OR driver_id == '%s')", order.id, order.main_vehicle_number.c_str(), order.driver_id.c_str());
    if (exist_record)
    {
        ret = true;
    }

    return ret;
}

static bool pri_create_order(const std::vector<vehicle_order_info> &orders, const std::string &ssid = "")
{
    bool ret = false;
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, false);
    if (!opt_user)
    {
        if (orders.size() > 0)
        {
            auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", orders[0].company_name.c_str());
            opt_user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
        }
    }
    if (!opt_user && ssid.length() > 0)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    for (auto itr = orders.begin(); itr != orders.end(); ++itr)
    {
        auto find_begin_ret = std::find_if(
            orders.begin(), itr,
            [&](const vehicle_order_info &_val)
            {
                return (itr->main_vehicle_number == _val.main_vehicle_number || itr->driver_id == _val.driver_id);
            });
        if (find_begin_ret != itr)
        {
            ZH_RETURN_DUP_ORDER();
        }
        auto find_end_ret = std::find_if(
            itr + 1, orders.end(),
            [&](const vehicle_order_info &_val)
            {
                return (itr->main_vehicle_number == _val.main_vehicle_number || itr->driver_id == _val.driver_id);
            });

        if (find_end_ret != orders.end())
        {
            ZH_RETURN_DUP_ORDER();
        }
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
        if (tmp.driver_id.length() > 0 && tmp.driver_id[tmp.driver_id.length() - 1] == 'X')
        {
            tmp.driver_id[tmp.driver_id.length() - 1] = 'x';
        }
        tmp.driver_name = order.driver_name;
        tmp.driver_phone = order.driver_phone;
        tmp.main_vehicle_number = order.main_vehicle_number;
        tmp.stuff_name = order.stuff_name;
        tmp.company_name = order.company_name;
        tmp.status = -1;
        tmp.company_address = order.company_address;
        tmp.use_for = order.use_for;
        tmp.max_count = order.max_count;
        tmp.end_time = order.end_time;
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", order.stuff_name.c_str());
        if (stuff && stuff->need_enter_weight)
        {
            tmp.need_enter_weight = 1;
        }

        bool op_permit = true;
        if (opt_user)
        {
            auto contract = opt_user->get_parent<zh_sql_contract>("belong_contract");
            if (contract)
            {
                if (contract->name != tmp.company_name)
                {
                    op_permit = false;
                }
            }
        }

        if (op_permit && tmp.insert_record())
        {
            tmp.order_number = std::to_string(time(nullptr)) + std::to_string(tmp.get_pri_id());
            auto create_status = zh_sql_order_status::make_create_status(ssid);
            change_order_status(tmp, create_status);
            bool auto_confirm = false;
            if (ssid.length() > 0)
            {
                auto_confirm = system_management_handler::get_inst()->is_auto_confirm(ssid);
            }
            if (!opt_user || !opt_user->get_parent<zh_sql_contract>("belong_contract") || auto_confirm)
            {
                auto before_come_status = zh_sql_order_status::make_before_come_status();
                change_order_status(tmp, before_come_status);
            }
            ret = tmp.update_record();
        }
    }

    return ret;
}

static void dup_one_order(zh_sql_vehicle_order &vo)
{
    if (vo.status == 100 && vo.end_time.length() > 0)
    {
        auto now_date = zh_rpc_util_get_datestring().substr(0, 10);
        auto end_date = vo.end_time.substr(0, 10);
        if (now_date == end_date)
        {
            vehicle_order_info tmp_order;
            tmp_order.behind_vehicle_number = vo.behind_vehicle_number;
            tmp_order.driver_id = vo.driver_id;
            tmp_order.driver_name = vo.driver_name;
            tmp_order.driver_phone = vo.driver_phone;
            tmp_order.main_vehicle_number = vo.main_vehicle_number;
            tmp_order.stuff_name = vo.stuff_name;
            tmp_order.company_name = vo.company_name;
            tmp_order.company_address = vo.company_address;
            tmp_order.use_for = vo.use_for;
            tmp_order.max_count = vo.max_count;
            tmp_order.end_time = vo.end_time;

            std::vector<vehicle_order_info> tmp;
            tmp.push_back(tmp_order);
            pri_create_order(tmp);
        }
    }
}
bool vehicle_order_center_handler::create_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &orders)
{
    auto ssid_verify = ssid;
    if (ssid_verify.length() <= 0)
    {
        ssid_verify = "xxx";
    }
    return pri_create_order(orders, ssid_verify);
}

bool vehicle_order_center_handler::confirm_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order)
{
    bool ret = true;
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, false);

    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_ORDER);
    }

    for (auto &itr : order)
    {
        auto single_order = sqlite_orm::search_record<zh_sql_vehicle_order>(itr.id);
        if (single_order)
        {
            auto tmp = zh_sql_order_status::make_before_come_status(ssid);
            change_order_status(*single_order, tmp);
        }
    }

    return ret;
}

bool vehicle_order_center_handler::cancel_vehicle_order(const std::string &ssid, const std::vector<vehicle_order_info> &order)
{
    bool ret = true;

    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, false);
    if (!opt_user)
    {
        if (order.size() > 0)
        {
            auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", order[0].company_name.c_str());
            opt_user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
        }
    }
    bool op_by_drvier = false;
    if (!opt_user)
    {
        auto op_by_driver_order = sqlite_orm::search_record<zh_sql_vehicle_order>("driver_phone == '%s'", ssid.c_str());
        if (op_by_driver_order)
        {
            op_by_drvier = true;
        }
        else
        {
            ZH_RETURN_NO_PRAVILIGE();
        }
    }

    std::list<zh_sql_vehicle_order> need_cancel;
    for (auto &itr : order)
    {
        auto single_order = sqlite_orm::search_record<zh_sql_vehicle_order>(itr.id);
        if (!single_order || (single_order->status >= 2 && single_order->status != 100))
        {
            ZH_RETURN_ORDER_CANNOT_CANCEL(itr.main_vehicle_number);
        }
        need_cancel.push_back(*single_order);
    }

    for (auto &itr : need_cancel)
    {
        if (op_by_drvier)
        {
            auto cancel_status = zh_sql_order_status::make_end_status();
            change_order_status(itr, cancel_status);
        }
        else
        {
            auto cancel_status = zh_sql_order_status::make_end_status(ssid);
            change_order_status(itr, cancel_status);
        }
    }

    return ret;
}

std::shared_ptr<scale_state_machine> vehicle_order_center_handler::get_scale_sm(const std::string &_name)
{
    return ssm_map[_name];
}

void vehicle_order_center_handler::get_order_detail(vehicle_order_detail &_return, const std::string &ssid, const std::string &order_number)
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", order_number.c_str());
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    make_vehicle_detail_from_sql(_return, *vo);
}
bool vehicle_order_center_handler::confirm_order_deliver(const std::string &ssid, const std::string &order_number, const bool confirmed)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", order_number.c_str());
    if (!vo || vo->status >= 4)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    vo->m_permit = confirmed;
    ret = vo->update_record();
    return ret;
}

bool vehicle_order_center_handler::update_vehicle_order(const std::string &ssid, const vehicle_order_info &order)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, false);
    if (!user)
    {
        auto contract = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", order.company_name.c_str());
        if (contract)
        {
            user.reset(zh_rpc_util_get_online_user(ssid, *contract).release());
        }
    }
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order.id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    if (vo->status > 1)
    {
        ZH_RETURN_MSG("派车单正在运行或已关闭，不允许编辑");
    }
    vo->behind_vehicle_number = order.behind_vehicle_number;
    vo->company_name = order.company_name;
    vo->driver_id = order.driver_id;
    vo->driver_name = order.driver_name;
    vo->driver_phone = order.driver_phone;
    vo->main_vehicle_number = order.main_vehicle_number;
    vo->stuff_name = order.stuff_name;
    if (order.attachment.length() > 0)
    {
        auto exist_attachment = vo->get_parent<zh_sql_file>("attachment");
        if (!exist_attachment || exist_attachment->name != order.attachment)
        {
            zh_sql_file attach_file;
            attach_file.save_file(order.attachment.substr(0, order.attachment.find_last_of('.')), order.order_number + "-" + std::to_string(time(nullptr)) + order.attachment.substr(order.attachment.find_last_of('.'), order.attachment.length()));
            attach_file.insert_record();
            vo->set_parent(attach_file, "attachment");
        }
    }
    else
    {
        auto exist_attachment = vo->get_parent<zh_sql_file>("attachment");
        if (exist_attachment)
        {
            exist_attachment->remove_record();
        }
        zh_sql_file empty;
        vo->set_parent(empty, "attachment");
    }
    ret = vo->update_record();
    return ret;
}
static bool valid_stay_limit(zh_sql_vehicle_order &_order)
{
    bool ret = true;

    auto smh = system_management_handler::get_inst();
    if (smh)
    {
        register_config_info rci;
        smh->get_register_info(rci);
        if (rci.enabled && rci.leave_limit > 0 && _order.m_cam_time.length() > 0)
        {
            auto latest_leave_time = zh_rpc_util_get_time_by_string(_order.m_cam_time) + rci.leave_limit * 60;
            if (time(nullptr) > latest_leave_time)
            {
                auto end_status = zh_sql_order_status::make_end_status();
                end_status.user_name = "超时自动";
                change_order_status(_order, end_status);
                ret = false;
            }
        }
    }

    return ret;
}
static bool vehicle_leave_enough(const std::string &_vehicle_number)
{
    bool ret = true;

    auto smh = system_management_handler::get_inst();
    if (smh)
    {
        register_config_info rci;
        smh->get_register_info(rci);
        if (rci.enabled && rci.check_in_time > 0)
        {
            auto expect_leave_time = time(nullptr) - rci.check_in_time * 60;
            auto last_leave_vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status == 100 AND exit_cam_time != '' ORDER BY PRI_ID DESC LIMIT 1", _vehicle_number.c_str());
            if (last_leave_vo)
            {
                auto last_leave_time = zh_rpc_util_get_time_by_string(last_leave_vo->exit_cam_time);
                if (last_leave_time > expect_leave_time)
                {
                    ret = false;
                }
            }
        }
    }

    return ret;
}

static bool pri_calcu_balanc(zh_sql_contract &_company, zh_sql_stuff &_stuff, int _vehicle_count)
{
    bool ret = false;

    double single_price = _stuff.price;
    auto bound_price = sqlite_orm::search_record<zh_sql_contract_stuff_price>("customer_name == '%s' AND stuff_name == '%s'", _company.name.c_str(), _stuff.name.c_str());
    if (bound_price)
    {
        single_price = bound_price->price;
    }

    auto requier_cash = _vehicle_count * _stuff.expect_weight * single_price;
    if (requier_cash <= _company.balance + _company.credit)
    {
        ret = true;
    }

    return ret;
}

bool vehicle_order_center_handler:: driver_check_in(const int64_t order_id, const bool is_cancel, const std::string& driver_id)
{
    bool ret = false;
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    if (vo->status != 1)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    if (!is_cancel && vo->need_enter_weight && (vo->enter_weight == 0 || !vo->get_parent<zh_sql_file>("enter_weight_attachment")))
    {
        ZH_RETURN_MSG("请先填写进厂前净重并上传矿（厂）发磅单");
    }
    if (!is_cancel && !vehicle_leave_enough(vo->main_vehicle_number))
    {
        ZH_RETURN_MSG("离场时间过短，无法排号，请稍后排号");
    }
    vo->m_registered = is_cancel ? 0 : 1;
    if (!vo->m_registered)
    {
        vo->check_in_timestamp = 0;
        if (vo->m_called)
        {
            auto pmh = plugin_management_handler::get_inst();
            if (pmh)
            {
                plugin_event_info pei;
                pei.type = plugin_event_info::cancel_call_driver;
                pei.order_number = vo->order_number;
                pmh->deliver_event(pei);
            }
            vo->m_called = 0;
            vo->call_timestamp = 0;
            std::string oem_name;
            system_management_handler::get_inst()->get_oem_name(oem_name);
            std::string sms_cmd = "python3 /script/send_sms.py '" + vo->driver_phone + "' '" + vo->driver_name + "' '" + oem_name + "取消'";
            tdf_log tmp_log("sms");
            tmp_log.log(sms_cmd);
            system(sms_cmd.c_str());
        }
    }
    else
    {
        vo->check_in_timestamp = time(nullptr);
        auto pmh = plugin_management_handler::get_inst();
        if (pmh)
        {
            std::string std_out;
            std::string std_err;
            auto stuff_info = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
            auto customer_info = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", vo->company_name.c_str());
            if (plugin_is_installed("zh_hnnc"))
            {
                if (stuff_info && customer_info && stuff_info->code.length() > 0 && customer_info->code.length() > 0)
                {
                    vo->update_record();
                    auto related_vehicle = sqlite_orm::search_record_all<zh_sql_vehicle_order>("company_name == '%s' AND stuff_name == '%s' AND m_registered == 1 AND status != 100", customer_info->name.c_str(), stuff_info->name.c_str());
                    if (customer_info->is_sale)
                    {
                        pmh->zh_plugin_run_plugin("valid_balance " + customer_info->code + " " + stuff_info->code + " " + std::to_string(related_vehicle.size()), "zh_hnnc", std_out, std_err);
                        if (std_err.length() > 0)
                        {
                            vo->m_registered = 0;
                            vo->check_in_timestamp  = 0;
                            vo->update_record();
                            ZH_RETURN_MSG("余额不足，无法排号，请联系货主充值");
                        }
                    }
                    else
                    {
                        pmh->zh_plugin_run_plugin("valid_remain " + customer_info->code + " " + stuff_info->code + " " + std::to_string(related_vehicle.size()), "zh_hnnc", std_out, std_err);
                        if (std_err.length() > 0)
                        {
                            std::string err_info = "订单余量不足，请联系" + std::string(getenv("OEM_SHORT")) + "修改";
                            vo->m_registered = 0;
                            vo->check_in_timestamp  = 0;
                            vo->update_record();
                            ZH_RETURN_MSG("余额不足，无法排号，请联系货主充值");
                            ZH_RETURN_MSG(err_info);
                        }
                    }
                }
            }
            if (plugin_is_installed("zh_meiyitong"))
            {
                pmh->zh_plugin_run_plugin("need_proc " + vo->stuff_name, "zh_meiyitong", std_out, std_err);
                if (std_err.length() == 0)
                {
                    pmh->zh_plugin_run_plugin(
                        "check_in " +
                            vo->order_number.substr(vo->order_number.length() - 11, 11) + " " +
                            vo->company_name + " " +
                            vo->stuff_name + " " +
                            vo->main_vehicle_number,
                        "zh_meiyitong", std_out, std_err);
                    if (std_err.length() > 0)
                    {
                        ZH_RETURN_MSG("上传监管平台接单失败:" + std_err);
                    }
                }
            }
            if (stuff_info && customer_info && customer_info->is_sale && stuff_info->code.empty())
            {
                auto related_vehicle = sqlite_orm::search_record_all<zh_sql_vehicle_order>("company_name == '%s' AND stuff_name == '%s' AND m_registered == 1 AND status != 100", customer_info->name.c_str(), stuff_info->name.c_str());
                if (!pri_calcu_balanc(*customer_info, *stuff_info, related_vehicle.size() + 1))
                {
                    ZH_RETURN_MSG("余额不足，无法排号，请联系货主充值");
                }
            }
        }
    }
    if (driver_id.length() > 0)
    {
        vo->driver_id = driver_id;
    }
    ret = vo->update_record();

    return ret;
}

void vehicle_order_center_handler::driver_get_order(vehicle_order_detail &_return, const std::string &order_number)
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("driver_phone == '%s' AND status != 100 AND status != 0", order_number.c_str());
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    auto order_before = sqlite_orm::search_record_all<zh_sql_vehicle_order>("stuff_name == '%s' AND check_in_timestamp <= %ld AND m_registered == 1 AND status == 1 AND m_called == 0", vo->stuff_name.c_str(), vo->check_in_timestamp);
    make_vehicle_detail_from_sql(_return, *vo);
    _return.wait_count = order_before.size() - 1;
    if (vo->m_called)
    {
        _return.wait_count = -1;
    }
}

bool vehicle_order_center_handler::pri_call_vehicle(const int64_t order_id, const bool is_cancel, const std::string &_user_name)
{
    bool ret = false;
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    if (vo->status != 1)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    if (!vo->m_registered)
    {
        ZH_RETURN_MSG("车辆未排号或已取消排号");
    }
    auto orig_called = vo->m_called;
    vo->m_called = is_cancel ? 0 : 1;
    plugin_event_info pei;
    pei.order_number = vo->order_number;
    if (is_cancel)
    {
        vo->call_timestamp = 0;
        pei.type = plugin_event_info::cancel_call_driver;
    }
    else
    {
        vo->call_timestamp = time(nullptr);
        vo->call_user_name = _user_name;
        pei.type = plugin_event_info::call_driver;
    }
    ret = vo->update_record();
    if (ret && vo->m_called && orig_called != vo->m_called)
    {
        auto pmh = plugin_management_handler::get_inst();
        if (pmh)
        {
            pmh->deliver_event(pei);
        }
        std::string oem_name;
        system_management_handler::get_inst()->get_oem_name(oem_name);
        std::string sms_cmd = "python3 /script/send_sms.py '" + vo->driver_phone + "' '" + vo->driver_name + "' '" + oem_name + "'";
        tdf_log tmp_log("sms");
        tmp_log.log(sms_cmd);
        if (0 != system(sms_cmd.c_str()))
        {
            ZH_RETURN_MSG("发送叫号短信失败");
        }
    }

    return ret;
}
bool vehicle_order_center_handler::call_vehicle(const std::string &ssid, const int64_t order_id, const bool is_cancel)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    return pri_call_vehicle(order_id, is_cancel, user->name);
}

void vehicle_order_center_handler::get_registered_vehicle(std::vector<vehicle_order_detail> &_return, const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD, true);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    auto vos = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status >= 1 AND status != 100 AND m_registered == 1 ORDER BY check_in_timestamp");
    for (auto &itr : vos)
    {
        vehicle_order_detail tmp;
        make_vehicle_detail_from_sql(tmp, itr);
        _return.push_back(tmp);
    }
}

bool vehicle_order_center_handler::manual_set_p_weight(const std::string &ssid, const int64_t order_id, const double weight)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }

    auto status = zh_sql_order_status::make_p_status(ssid);
    change_order_status(*vo, status);
    vo->p_weight = weight;
    ret = vo->update_record();

    return ret;
}
bool vehicle_order_center_handler::manual_set_m_weight(const std::string &ssid, const int64_t order_id, const double weight)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }

    auto status = zh_sql_order_status::make_m_status(ssid);
    change_order_status(*vo, status);
    vo->m_weight = weight;
    ret = vo->update_record();

    return ret;
}
static void recalcu_balance_inventory(zh_sql_vehicle_order &_vo, const std::string &_ssid = "")
{
    double single_price = 0;
    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", _vo.stuff_name.c_str());
    if (stuff)
    {
        stuff->inventory += _vo.p_weight - _vo.m_weight;
        if (_ssid.length() > 0)
        {
            stuff->update_record(_ssid);
        }
        else
        {
            stuff->update_record(_vo.order_number);
        }
    }
    auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", _vo.company_name.c_str());
    if (company && stuff && company->is_sale)
    {
        single_price = stuff->price;
        auto boud_price = sqlite_orm::search_record<zh_sql_contract_stuff_price>("customer_name == '%s' AND stuff_name == '%s'", company->name.c_str(), stuff->name.c_str());
        if (boud_price)
        {
            single_price = boud_price->price;
        }
        auto ch = contract_management_handler::get_inst();
        auto new_balance = company->balance - single_price * (_vo.m_weight - _vo.p_weight);
        ch->internal_change_balance(company->name, new_balance, "（系统自动）售出产品 " + stuff->name + " ：" + zh_double2string_reserve2(_vo.m_weight - _vo.p_weight) + stuff->unit);
        _vo.price = single_price;
        _vo.update_record();
    }
}
bool vehicle_order_center_handler::manual_close(const std::string &ssid, const int64_t order_id)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    auto status = zh_sql_order_status::make_end_status(ssid);
    auto save_hook = zh_order_save_hook([](zh_sql_vehicle_order &) -> bool
                                        { return true; },
                                        dup_one_order);
    change_order_status(*vo, status, save_hook);
    if (vo->m_cam_time.empty())
    {
        recalcu_balance_inventory(*vo, ssid);
    }

    ret = true;

    return ret;
}

void vehicle_order_center_handler::get_order_statistics(vehicle_order_statistics &_return, const std::string &ssid)
{
    std::string statis_cmd;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true);
    if (!user)
    {
        user.reset(zh_rpc_util_get_online_user(ssid, 3).release());
        if (!user)
        {
            ZH_RETURN_NO_PRAVILIGE();
        }
        statis_cmd = " AND company_name == '" + user->name + "'";
    }

    auto cvo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 0 %s", statis_cmd.c_str());
    auto convo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 1 %s", statis_cmd.c_str());
    auto evo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 2 %s", statis_cmd.c_str());
    auto fwvo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 3 %s", statis_cmd.c_str());
    auto swvo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 4 %s", statis_cmd.c_str());
    _return.created = cvo.size();
    _return.confirmed = convo.size();
    _return.entered = evo.size();
    _return.first_weight = fwvo.size();
    _return.second_weight = swvo.size();
    _return.total = _return.created + _return.confirmed + _return.entered + _return.second_weight + _return.first_weight;
}

bool vehicle_order_center_handler::upload_enter_weight_attachment(const int64_t order_id, const std::string &attachment, const double enter_weight)
{
    bool ret = false;
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    if (vo->status != 1)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    if (attachment.length() > 0)
    {
        auto exist_attachment = vo->get_parent<zh_sql_file>("enter_weight_attachment");
        if (!exist_attachment || exist_attachment->name != attachment)
        {
            zh_sql_file attach_file;
            attach_file.save_file(attachment.substr(0, attachment.find_last_of('.')), vo->order_number + "-" + std::to_string(time(nullptr)) + attachment.substr(attachment.find_last_of('.'), attachment.length()));
            attach_file.insert_record();
            vo->set_parent(attach_file, "enter_weight_attachment");
        }
    }
    vo->enter_weight = enter_weight;
    ret = vo->update_record();

    return ret;
}

bool vehicle_order_center_handler::print_weight_ticket(const std::string &ssid, const int64_t order_id, const std::string &scale_name)
{
    bool ret = false;

    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_FIELD, true);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_FIELD);
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    auto ssm = get_scale_sm(scale_name);
    if (ssm)
    {
        ssm->print_weight_ticket(vo);
        ret = true;
    }

    return ret;
}

void vehicle_order_center_handler::check_price_balance(std::string &_return, const std::string &ssid, const std::vector<vehicle_order_info> &order)
{
    auto user = zh_rpc_util_get_online_user(ssid);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    std::string ret;
    if (order.size() > 0)
    {
        auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", order[0].company_name.c_str());
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", order[0].stuff_name.c_str());
        if (company && stuff)
        {
            if (company->is_sale)
            {
                double already_cost = 0;
                if (!balance_enough(*company, *stuff, order.size(), &already_cost))
                {
                    auto already_exist_vehicle_count = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status != 100 AND company_name == '%s'", company->name.c_str()).size();
                    if (already_exist_vehicle_count > 0)
                    {
                        ret = "已经派车" + std::to_string(already_exist_vehicle_count) + "辆,预计花费" + zh_double2string_reserve2(already_cost) + "元,";
                    }
                    ret += "余额(" + zh_double2string_reserve2(company->balance) + ")不足, 若新增派车" + std::to_string(order.size()) + "辆，余额需要增加" + zh_double2string_reserve2(stuff->price * order.size() * stuff->expect_weight - company->credit - company->balance + already_cost) + "元";
                }
            }
        }
        else
        {
            ret = "系统未维护派车公司或货品，无法检查价格或余额";
        }
    }

    _return = ret;
}

scale_state_machine::scale_state_machine(const device_scale_config &_config)
    : m_log(_config.name + " scale sm"),
      bound_scale(_config),
      entry_id_api(bound_scale.entry_id_reader_ip, ZH_ID_READER_PORT,
                   [this](const std::string &_id)
                   {
                       this->proc_trigger_id_read(_id, this->bound_scale.entry_id_reader_ip);
                       system_management_handler::get_inst()->id_result[this->bound_scale.entry_id_reader_ip] = _id;
                       this->trigger_sm();
                   }),
      exit_id_api(bound_scale.exit_id_reader_ip, ZH_ID_READER_PORT,
                  [this](const std::string &_id)
                  {
                      this->proc_trigger_id_read(_id, this->bound_scale.exit_id_reader_ip);
                      system_management_handler::get_inst()->id_result[this->bound_scale.exit_id_reader_ip] = _id;
                      this->trigger_sm();
                  })
{
    auto hk_call_back = [](const std::string &_plate_no, const std::string &_road_ip, void *_pdata)
    {
        system_management_handler::get_inst()->cam_result[_road_ip] = _plate_no;
        if (_plate_no.length() > 0)
        {
            auto pthis = (scale_state_machine *)_pdata;
            pthis->proc_trigger_vehicle(_plate_no, _road_ip);
            pthis->trigger_sm();
        }
    };
    auto zh_qr_callback = [](const std::string &_qr_code, const std::string &_qr_ip, void *_pdata)
    {
        system_management_handler::get_inst()->qr_result[_qr_ip] = _qr_code;
        if (_qr_code.length() > 0)
        {
            auto pthis = (scale_state_machine *)_pdata;
            pthis->proc_trigger_qr(_qr_code, _qr_ip);
            pthis->trigger_sm();
        }
    };
    zh_sub_callback_cfg tmp_cfg;
    tmp_cfg.pData = this;
    tmp_cfg.callback = hk_call_back;
    if (_config.entry_config.cam_ip.length() > 0)
    {
        zh_hk_subcribe_event(_config.entry_config.cam_ip, tmp_cfg);
    }
    if (_config.exit_config.cam_ip.length() > 0)
    {
        zh_hk_subcribe_event(_config.exit_config.cam_ip, tmp_cfg);
    }
    tmp_cfg.callback = zh_qr_callback;
    if (_config.entry_qr_ip.length() > 0)
    {
        zh_qr_subscribe(_config.entry_qr_ip, tmp_cfg);
    }
    if (_config.exit_qr_ip.length() > 0)
    {
        zh_qr_subscribe(_config.exit_qr_ip, tmp_cfg);
    }
    m_cur_state.reset(new scale_sm_vehicle_come());
    zh_hk_cast_empty(_config.entry_config.led_ip);
    zh_hk_cast_empty(_config.exit_config.led_ip);
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
    zh_qr_unsubscribe(bound_scale.entry_qr_ip);
    zh_qr_unsubscribe(bound_scale.exit_qr_ip);
    reset_scale_connection(bound_scale.scale_ip, bound_scale.scale_brand);
}

void scale_state_machine::open_enter()
{
    m_log.log("开入口");
    zh_hk_ctrl_gate(enter_cam_ip, zh_hk_gate_open);
}
void scale_state_machine::open_exit()
{
    m_log.log("开出口");
    zh_hk_ctrl_gate(exit_cam_ip, zh_hk_gate_open);
}
void scale_state_machine::scale_zero()
{
    m_log.log("清零衡器");
    clean_scale_weight(bound_scale.scale_ip, ZH_SCALE_PORT, bound_scale.scale_brand);
}

static void continue_push_same_weight(std::vector<double> &_con, double _value)
{
    if (_value == 0)
    {
        _con.clear();
    }
    else
    {
        if (_con.size() > 0)
        {
            if (_con[0] != _value)
            {
                _con.clear();
            }
        }
    }
    _con.push_back(_value);
}

void scale_state_machine::open_scale_timer()
{
    m_log.log("开启定时器");
    manual_confirm_scale = false;
    timer_fd = tdf_main::get_inst().start_timer(
        3,
        [](void *_private)
        {
            auto ssm = (scale_state_machine *)_private;
            bool ready_for_scale = false;
            if (!raster_was_block(ssm->bound_scale.raster_ip[0], ZH_RASTER_PORT) && !raster_was_block(ssm->bound_scale.raster_ip[1], ZH_RASTER_PORT))
            {
                ready_for_scale = true;
                if (ssm->bound_scale.check_close)
                {
                    if (zh_hk_get_cam_IO(ssm->bound_scale.entry_config.cam_ip) && zh_hk_get_cam_IO(ssm->bound_scale.exit_config.cam_ip))
                    {
                        ready_for_scale = true;
                    }
                    else
                    {
                        ready_for_scale = false;
                    }
                }
            }
            if (ready_for_scale)
            {
                // 15848601177
                auto scale_ret = get_current_weight(ssm->bound_scale.scale_ip, ZH_SCALE_PORT, ssm->bound_scale.scale_brand);
                scale_ret *= ssm->bound_scale.coefficient;
                {
                    tdf_state_machine_lock a(*ssm);
                    continue_push_same_weight(ssm->continue_weight, scale_ret);
                }
                ssm->trigger_sm();
            }
            else if (ssm->m_cur_state->state_name() == "称重")
            {
                zh_hk_cast_raster_block(ssm->bound_scale.entry_config.led_ip);
                zh_hk_cast_raster_block(ssm->bound_scale.exit_config.led_ip);
            }
        },
        this);
}
void scale_state_machine::close_timer()
{
    m_log.log("关闭定时器");
    if (timer_fd >= 0)
    {
        tdf_main::get_inst().stop_timer(timer_fd);
        timer_fd = -1;
        continue_weight.clear();
        fin_weight = 0;
    }
    manual_confirm_scale = true;
}
void scale_state_machine::clean_bound_info()
{
    m_log.log("清理绑定数据");
    last_enter_cam_ip = enter_cam_ip;
    enter_cam_ip = "";
    exit_cam_ip = "";
    entry_param.clear();
    exit_param.clear();
    bound_vehicle_number = "";
}

bool scale_state_machine::should_open()
{
    bool ret = false;
    if ((entry_param.id_no == "" && entry_param.qr_code == "" && entry_param.vehicle_number == "") &&
        (exit_param.id_no == "" && exit_param.qr_code == "" && exit_param.vehicle_number == ""))
    {
        return false;
    }
    std::string tmp_vehicle_entry = ctrl_policy.pass_permit(entry_param.vehicle_number, entry_param.id_no, entry_param.qr_code);
    std::string tmp_vehicle_exit = ctrl_policy.pass_permit(exit_param.vehicle_number, exit_param.id_no, exit_param.qr_code);
    auto cur_vehicle = tmp_vehicle_entry.length() > 0 ? tmp_vehicle_entry : tmp_vehicle_exit;
    auto led_ip = tmp_vehicle_entry.length() > 0 ? bound_scale.entry_config.led_ip : bound_scale.exit_config.led_ip;
    if (cur_vehicle.length() > 0)
    {
        auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", cur_vehicle.c_str());
        if (vo)
        {
            if (vo->m_called)
            {
                ret = true;
                if (vo->status == 3 && !vo->m_permit)
                {
                    ret = false;
                }
            }
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

bool scale_state_machine::assume_stable_considering_manual(double _weight)
{
    bool ret = true;

    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100 AND status > 0", bound_vehicle_number.c_str());
    if (vo && vo->status == 3)
    {
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
        if (stuff && stuff->need_manual_scale)
        {
            ret = false;
        }
    }
    if (false == ret)
    {
        ret = manual_confirm_scale;
    }

    return ret;
}

bool scale_state_machine::scale_stable()
{
    bool ret = false;
    if (continue_weight.size() > 2)
    {
        auto cur_weight = [=]() -> double
        {
            double scale_ret = 0;
            for (auto &itr : continue_weight)
            {
                scale_ret += itr;
            }
            if (continue_weight.size() > 0)
            {
                scale_ret /= continue_weight.size();
            }
            return scale_ret;
        }();
        if (cur_weight >= bound_scale.min_weight)
        {
            if (assume_stable_considering_manual(cur_weight))
            {
                ret = true;
            }
            else
            {
                auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100 AND status > 0", bound_vehicle_number.c_str());
                if (vo)
                {
                    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
                    if (stuff && !lack_weight)
                    {
                        if (cur_weight > stuff->max_limit)
                        {
                            zh_hk_cast_need_drop(bound_scale.entry_config.led_ip, bound_vehicle_number);
                            zh_hk_cast_need_drop(bound_scale.exit_config.led_ip, bound_vehicle_number);
                            ret = true;
                        }
                        else if (cur_weight < stuff->min_limit)
                        {
                            lack_weight = true;
                            zh_hk_cast_need_manual(bound_scale.entry_config.led_ip, bound_vehicle_number);
                            zh_hk_cast_need_manual(bound_scale.exit_config.led_ip, bound_vehicle_number);
                        }
                        else
                        {
                            ret = true;
                        }
                    }
                }
            }
        }
    }

    return ret;
}
bool scale_state_machine::scale_clear()
{
    bool ret = false;
    if (continue_weight.size() > 0)
    {
        auto cur_weight = continue_weight[continue_weight.size() - 1];
        if (cur_weight < 1)
        {
            ret = true;
        }
    }

    return ret;
}

static bool nvr_info_valided(const device_scale_config &_scale_config)
{
    bool ret = false;

    auto &entry_login = _scale_config.entry_login;
    auto &exit_login = _scale_config.exit_login;
    auto &scale1 = _scale_config.scale1;
    auto &scale2 = _scale_config.scale2;
    auto &scale3 = _scale_config.scale3;
    auto login_is_valided = [](const nvr_login_info &_login) -> bool
    {
        bool ret = false;
        if (_login.username.length() > 0 && _login.password.length() > 0)
        {
            ret = true;
        }

        return ret;
    };
    if (login_is_valided(entry_login) &&
        login_is_valided(exit_login) &&
        login_is_valided(scale1) &&
        login_is_valided(scale2) &&
        login_is_valided(scale3))
    {
        ret = true;
    }

    return ret;
}

static std::string order_number2voc_number(const std::string &_order_number)
{
    std::string ret;
    ret = _order_number.substr(_order_number.length() - 11, 11);

    return ret;
}

static void change_json_key(neb::CJsonObject &_json, const std::string &_orig_key, const std::string &_new_key)
{
    if (_json.KeyExist(_orig_key))
    {
        auto json_value = _json(_orig_key);
        _json.Delete(_orig_key);
        _json.Add(_new_key, json_value);
    }
}
static NET_DVR_TIME dateTime2unix(const std::string &_time, int _min = 0)
{
    NET_DVR_TIME tm;
    memset(&tm, 0, sizeof(tm));
    auto timeStamp = _time;
    if (_min != 0)
    {
        auto time_sec = zh_rpc_util_get_time_by_string(_time) - _min;
        timeStamp = zh_rpc_util_get_timestring(time_sec);
    }
    sscanf(
        timeStamp.c_str(), "%d-%d-%d %d:%d:%d",
        &tm.dwYear, &tm.dwMonth, &tm.dwDay,
        &tm.dwHour, &tm.dwMinute, &tm.dwSecond);

    return tm;
}
struct zh_rest_api_meta
{
    neb::CJsonObject req;
    std::string order_number;
    std::function<bool(const neb::CJsonObject &, const std::string &)> proc_func;
    bool is_running = false;
    zh_rest_api_meta(const neb::CJsonObject &_req, const std::string &_order_number, const std::function<bool(const neb::CJsonObject &, const std::string &)> _proc_func) : req(_req), order_number(_order_number), proc_func(_proc_func) {}
    zh_rest_api_meta() {}
};
struct zh_rest_api_req_pipe
{
    std::list<zh_rest_api_meta> rest_que;
    pthread_mutex_t read_lock;
    pthread_cond_t que_cond = PTHREAD_COND_INITIALIZER;
    tdf_log m_log;
    void cancel_que(bool _is_running = false)
    {
        pthread_mutex_lock(&read_lock);
        auto &top_req = rest_que.front();
        bool need_delete = false;
        if (_is_running)
        {
            if (top_req.is_running)
            {
                need_delete = true;
            }
        }
        else
        {
            need_delete = true;
        }
        if (need_delete)
        {
            m_log.log("pop req in, order_number:%s,req:%s", top_req.order_number.c_str(), top_req.req.ToFormattedString().c_str());
            rest_que.pop_front();
        }
        pthread_mutex_unlock(&read_lock);
    }
    zh_rest_api_req_pipe() : m_log("rest_api")
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&read_lock, &attr);
        std::thread(
            [&]()
            {
                while (1)
                {
                    zh_rest_api_meta cur_node;
                    pthread_mutex_lock(&read_lock);
                    timeval now;
                    timespec outtime;
                    gettimeofday(&now, NULL);
                    outtime.tv_sec = now.tv_sec + 5;
                    outtime.tv_nsec = now.tv_usec * 1000;
                    m_log.log("wait signal");
                    pthread_cond_timedwait(&que_cond, &read_lock, &outtime);
                    m_log.log("timeout or something happened");
                    if (rest_que.size() > 0)
                    {
                        rest_que.front().is_running = true;
                        cur_node = rest_que.front();
                    }
                    pthread_mutex_unlock(&read_lock);
                    if (cur_node.is_running && cur_node.proc_func(cur_node.req, cur_node.order_number))
                    {
                        cancel_que(true);
                    }
                }
            })
            .detach();
    }
    void push_req(const zh_rest_api_meta &_req)
    {
        pthread_mutex_lock(&read_lock);
        m_log.log("push req in, order_number:%s,req:%s", _req.order_number.c_str(), _req.req.ToFormattedString().c_str());
        rest_que.push_back(_req);
        pthread_cond_signal(&que_cond);
        pthread_mutex_unlock(&read_lock);
    }
    std::list<std::string> go_throw_que()
    {
        pthread_mutex_lock(&read_lock);
        std::list<std::string> ret;
        for (auto &itr : rest_que)
        {
            ret.push_back(itr.order_number + '-' + itr.req.ToString());
        }
        pthread_mutex_unlock(&read_lock);
        return ret;
    }
};
static zh_rest_api_req_pipe g_nc_req_pip;
static bool push_req_to_ordos_ticket(zh_sql_vehicle_order &_order)
{
    bool ret = true;

    if (_order.status < 3)
    {
        g_nc_req_pip.push_req(
            zh_rest_api_meta(
                neb::CJsonObject(),
                _order.order_number,
                [](const neb::CJsonObject &, const std::string &_order_number) -> bool
                {
                    sleep(1);
                    bool push_p_ret = false;
                    auto pmh = plugin_management_handler::get_inst();
                    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", _order_number.c_str());
                    if (vo && pmh)
                    {
                        std::string std_out;
                        std::string std_err;
                        pmh->zh_plugin_run_plugin(
                            "p_weight " +
                                vo->main_vehicle_number + " " +
                                zh_double2string_reserve2(vo->p_weight),
                            "zh_ordos_ticket", std_out, std_err);
                        if (std_err.length() > 0)
                        {
                            vo->err_string = "外挂过皮失败：" + std_err;
                        }
                        else
                        {
                            push_p_ret = true;
                            vo->err_string = "";
                        }
                        vo->update_record("err_string");
                    }
                    return push_p_ret;
                }));
    }

    return ret;
}
static bool push_req_to_myt(zh_sql_vehicle_order &_order, const std::string &_nvr_ip, int _nvr_channel, const std::string &_username, const std::string &_password)
{
    bool ret = true;

    auto pmh = plugin_management_handler::get_inst();
    if (pmh)
    {
        std::string std_out;
        std::string std_err;
        pmh->zh_plugin_run_plugin("need_proc " + _order.stuff_name, "zh_meiyitong", std_out, std_err);
        if (std_err.length() > 0)
        {
            return ret;
        }
    }
    if (_order.status < 3)
    {
        g_nc_req_pip.push_req(
            zh_rest_api_meta(
                neb::CJsonObject(),
                _order.order_number,
                [](const neb::CJsonObject &, const std::string &_order_number) -> bool
                {
                    sleep(1);
                    bool push_p_ret = false;
                    auto pmh = plugin_management_handler::get_inst();
                    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", _order_number.c_str());
                    if (vo && pmh)
                    {
                        std::string std_out;
                        std::string std_err;
                        pmh->zh_plugin_run_plugin(
                            "push_p " +
                                order_number2voc_number(vo->order_number) + " " +
                                vo->main_vehicle_number + " " +
                                zh_double2string_reserve2(vo->p_weight),
                            "zh_meiyitong", std_out, std_err);
                        if (std_err.empty())
                        {
                            push_p_ret = true;
                            neb::CJsonObject tmp_extra(vo->extra_info);
                            tmp_extra.ReplaceAdd("VehicleTareID", std_out);
                            vo->extra_info = tmp_extra.ToString();
                        }
                        else
                        {
                            vo->err_string = "上传监管平台失败：" + std_err;
                        }
                        vo->update_record("err_string");
                        vo->update_record("extra_info");
                    }

                    return push_p_ret;
                }));
    }
    else if (_order.status < 4)
    {
        neb::CJsonObject m_req;
        m_req.Add("VehiclePlate", _order.main_vehicle_number);
        m_req.Add("GrossWeight", zh_double2string_reserve2(_order.m_weight));
        m_req.Add("CustomerName", _order.company_name);
        auto sd_record = sqlite_orm::search_record<zh_sql_stuff_source_dest>("name == '%s' AND is_source == 0", _order.source_dest_name.c_str());
        if (sd_record)
        {
            m_req.Add("DivisionID", sd_record->id);
        }
        m_req.Add("DivisionName", _order.source_dest_name);
        g_nc_req_pip.push_req(
            zh_rest_api_meta(
                m_req,
                _order.order_number,
                [=](const neb::CJsonObject &_req, const std::string &_order_number) -> bool
                {
                    sleep(1);

                    auto p_video = zh_hk_get_channel_video(_nvr_ip, _nvr_channel, dateTime2unix(_order.p_cam_time), dateTime2unix(_order.p_cam_time, 15), _username, _password);
                    auto m_video = zh_hk_get_channel_video(_nvr_ip, _nvr_channel, dateTime2unix(_order.m_cam_time), dateTime2unix(_order.m_cam_time, 15), _username, _password);
                    bool push_m_ret = false;
                    auto pmh = plugin_management_handler::get_inst();
                    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", _order_number.c_str());
                    if (pmh && vo)
                    {
                        std::string std_out;
                        std::string std_err;
                        auto tmp_req = _req;
                        neb::CJsonObject tmp_extra(vo->extra_info);
                        tmp_req.Add("VehicleTareID", tmp_extra("VehicleTareID"));
                        tmp_req.Add("TareVideoUrl", p_video);
                        tmp_req.Add("GrossVideoUrl", m_video);
                        tmp_req.Add("ProductName", vo->stuff_name);

                        pmh->zh_plugin_run_plugin("push_m '" + tmp_req.ToString() + "'", "zh_meiyitong", std_out, std_err);
                        if (std_err.empty())
                        {
                            push_m_ret = true;
                            vo->err_string = "";
                        }
                        else
                        {
                            vo->err_string = "上传监管平台失败：" + std_err;
                        }
                        vo->update_record("err_string");
                    }
                    return push_m_ret;
                }));
    }

    return ret;
}

static bool push_req_to_hn(zh_sql_vehicle_order &_order, const std::string &_pic1, const std::string &_pic2, const std::string &_pic3)
{
    bool ret = false;
    neb::CJsonObject req;

    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", _order.stuff_name.c_str());
    auto customer = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", _order.company_name.c_str());
    if (stuff && customer)
    {
        req.Add("Invcode", stuff->code);
        req.Add("Custcode", customer->code);
        req.Add("OTHbillcode", _order.order_number);
        req.Add("Carno", _order.main_vehicle_number);
        if (_order.status < 3)
        {
            if (customer->is_sale)
            {
                req.Add("Emptweight", _order.p_weight);
                req.Add("transtype", "UpTare");
                req.Add("PicurlT1", _pic1);
                req.Add("PicurlT2", _pic2);
                req.Add("PicurlT3", _pic3);
                req.Add("LoadbillNo", _order.order_number.substr(_order.order_number.length() - 11, 11));
            }
            else
            {
                req.Add("Grossweight", _order.p_weight);
                req.Add("transtype", "UpGrossRece");
                req.Add("PicurlG1", _pic1);
                req.Add("PicurlG2", _pic2);
                req.Add("PicurlG3", _pic3);
                req.Add("CarType", "N");
            }
            req.Add("Carlenth", 14);
            req.Add("CarBoxlenth", 12);
            req.Add("CarnormalWeiT", 15);
            req.Add("CarnormalWeiG", 50);
            req.Add("CarType", "N");
            req.Add("Caraxles", 6);

            req.Add("Sname", _order.driver_name);
            req.Add("Scode", _order.driver_id);
            req.Add("Sphone", _order.driver_phone);
        }
        else if (_order.status < 4)
        {
            if (customer->is_sale)
            {
                req.Add("LoadbillNo", _order.order_number.substr(_order.order_number.length() - 11, 11));
                req.Add("Emptweight", _order.p_weight);
                req.Add("transtype", "UpGross");
                req.Add("PicurlG1", _pic1);
                req.Add("PicurlG2", _pic2);
                req.Add("PicurlG3", _pic3);
                req.Add("Grossweight", _order.m_weight);
            }
            else
            {
                req.Add("transtype", "UpTareRece");
                req.Add("Emptweight", _order.m_weight);
                req.Add("PicurlT1", _pic1);
                req.Add("PicurlT2", _pic2);
                req.Add("PicurlT3", _pic3);
            }
            req.Add("IsRepeat", "N");
            req.Add("Netweight", std::abs(_order.m_weight - _order.p_weight));
            req.Add("Billcode", _order.bl_number);
            auto extra_info = neb::CJsonObject(_order.extra_info);
            if (extra_info.KeyExist("myt_info"))
            {
                ret = true;
            }
            if (std::abs(_order.m_weight - _order.p_weight) < 0.2)
            {
                req.Delete("IsRepeat");
                req.Delete("PicurlG3");
                req.Delete("PicurlG2");
                req.Delete("PicurlG1");
                req.Delete("Invcode");
                req.Delete("Custcode");
                req.Delete("LoadbillNo");
                req.ReplaceAdd("transtype", "UpEmpt");
                req.ReplaceAdd("Netweight", 0);
                req.ReplaceAdd("Grossweight", _order.p_weight);
                req.Add("PicurlE1", _pic1);
                req.Add("PicurlE2", _pic2);
                req.Add("PicurlE3", _pic3);
            }
        }
        if (stuff->code.length() > 0 && customer->code.length() > 0)
        {
            g_nc_req_pip.push_req(
                zh_rest_api_meta(
                    req,
                    _order.order_number,
                    [](const neb::CJsonObject &req_json_content, const std::string &_order_number) -> bool
                    {
                        sleep(1);
                        bool push_ret = false;
                        auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", _order_number.c_str());
                        auto pmh = plugin_management_handler::get_inst();
                        if (pmh && vo)
                        {
                            auto tmp_req = req_json_content;
                            if (req_json_content("transtype") == "UpGross" || req_json_content("transtype") == "UpEmpt" || req_json_content("transtype") == "UpTareRece")
                            {
                                tmp_req.ReplaceAdd("Billcode", vo->bl_number);
                            }
                            std::string std_out;
                            std::string std_err;
                            pmh->zh_plugin_run_plugin("push_req '" + tmp_req.ToString() + "'", "zh_hnnc", std_out, std_err);
                            vo.reset(sqlite_orm::search_record<zh_sql_vehicle_order>(vo->get_pri_id()).release());
                            if (vo)
                            {
                                if (std_err.empty())
                                {
                                    tdf_log tmp_log("plugin_exec");
                                    tmp_log.log(std_out);
                                    auto resp = neb::CJsonObject(std_out);
                                    tmp_log.log("bl:%s", resp("Billcode").c_str());
                                    tmp_log.log("nyj:%s", resp["NyjInf"].ToFormattedString().c_str());
                                    if (req_json_content("transtype") == "UpTare" || req_json_content("transtype") == "UpGrossRece")
                                    {
                                        vo->bl_number = resp("Billcode");
                                    }
                                    else if (req_json_content("transtype") == "UpGross" || req_json_content("transtype") == "UpEmpt")
                                    {
                                        neb::CJsonObject tmp_extra(vo->extra_info);
                                        tmp_extra.ReplaceAdd("myt_info", resp["NyjInf"].ToString());
                                        vo->extra_info = tmp_extra.ToString();
                                    }
                                    vo->err_string = "";
                                    push_ret = true;
                                }
                                else
                                {
                                    vo->err_string = "上传NC失败：" + std_err;
                                }
                                vo->update_record("err_string");
                                vo->update_record("extra_info");
                                vo->update_record("bl_number");
                            }
                        }
                        return push_ret;
                    }));
            ret = true;
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

std::unique_ptr<zh_sql_vehicle_order> scale_state_machine::record_order()
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", bound_vehicle_number.c_str());
    if (vo)
    {
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
        bool permit_m_weight = false;
        if (stuff)
        {
            if (stuff->need_manual_scale)
            {
                if ((stuff->max_limit >= fin_weight && stuff->min_limit <= fin_weight) || manual_confirm_scale)
                {
                    permit_m_weight = true;
                }
            }
            else
            {
                permit_m_weight = true;
            }
        }
        else
        {
            permit_m_weight = true;
        }
        auto save_hook = zh_order_save_hook(
            [&](zh_sql_vehicle_order &_order)
            {
                bool ret = true;
                if (plugin_is_installed("zh_hnnc"))
                {
                    if (nvr_info_valided(bound_scale))
                    {
                        auto pic1 = zh_hk_get_capture_picture(bound_scale.scale1_nvr_ip, bound_scale.scale1_channel, bound_scale.scale1.username, bound_scale.scale1.password);
                        auto pic2 = zh_hk_get_capture_picture(bound_scale.scale2_nvr_ip, bound_scale.scale2_channel, bound_scale.scale2.username, bound_scale.scale2.password);
                        auto pic3 = zh_hk_get_capture_picture(bound_scale.scale3_nvr_ip, bound_scale.scale3_channel, bound_scale.scale3.username, bound_scale.scale3.password);
                        ret = ret && push_req_to_hn(_order, pic1, pic2, pic3);
                    }
                }
                if (plugin_is_installed("zh_meiyitong"))
                {
                    auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s' AND is_sale == 1", _order.company_name.c_str());
                    if (company)
                    {
                        ret = ret && push_req_to_myt(_order, bound_scale.scale2_nvr_ip, bound_scale.scale2_channel, bound_scale.scale2.username, bound_scale.scale2.password);
                    }
                }
                if (plugin_is_installed("zh_ordos_ticket"))
                {
                    auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s' AND is_sale == 1", _order.company_name.c_str());
                    if (company)
                    {
                        ret = ret && push_req_to_ordos_ticket(_order);
                    }
                }
                _order.update_record();

                return ret;
            },
            [](zh_sql_vehicle_order &) {});
        if (vo->status < 3)
        {
            vo->p_nvr_ip1 = bound_scale.entry_nvr_ip + ":" + std::to_string(bound_scale.entry_channel);
            vo->p_nvr_ip2 = bound_scale.exit_nvr_ip + ":" + std::to_string(bound_scale.exit_channel);
            vo->p_cam_time = zh_rpc_util_get_timestring();
            auto status = zh_sql_order_status::make_p_status();
            vo->p_weight = fin_weight;
            change_order_status(*vo, status, save_hook);
        }
        else if (vo->status < 4 && permit_m_weight)
        {
            vo->m_nvr_ip1 = bound_scale.entry_nvr_ip + ":" + std::to_string(bound_scale.entry_channel);
            vo->m_nvr_ip2 = bound_scale.exit_nvr_ip + ":" + std::to_string(bound_scale.exit_channel);
            vo->m_cam_time = zh_rpc_util_get_timestring();
            auto status = zh_sql_order_status::make_m_status();
            vo->m_weight = fin_weight;
            change_order_status(*vo, status, save_hook);
            if (vo->status == 4)
            {
                recalcu_balance_inventory(*vo);
                if (!vo->get_children<zh_sql_order_status>("belong_order", "step == 2"))
                {
                    auto end_status = zh_sql_order_status::make_end_status();
                    auto save_hook_1 = zh_order_save_hook([](zh_sql_vehicle_order &) -> bool
                                                          { return true; },
                                                          dup_one_order);
                    change_order_status(*vo, end_status, save_hook_1);
                }
            }
        }
    }
    return vo;
}

void scale_state_machine::print_weight_ticket(const std::unique_ptr<zh_sql_vehicle_order> &vo)
{
    std::string content;
    std::string qr_code;
    if (vo)
    {
        content += "---------------\n";
        if (vo->bl_number.length() > 0)
        {
            content += "磅单号：" + vo->bl_number + "\n";
        }
        content += "称重车辆：" + vo->main_vehicle_number + "\n";
        content += "---------------\n";
        std::string m_weight_string = "未称重";
        std::string j_weight_string = "未知";
        std::string p_weight_string = m_weight_string;
        std::string p_time;
        std::string m_time;
        std::string p_type = "（毛重）：";
        std::string m_type = "（皮重）：";
        auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", vo->company_name.c_str());
        if (company && company->is_sale)
        {
            auto tmp_type = p_type;
            p_type = m_type;
            m_type = tmp_type;
        }
        auto p_status = vo->get_children<zh_sql_order_status>("belong_order", "step == 3");
        if (p_status)
        {
            p_time = p_status->timestamp;
        }
        auto m_status = vo->get_children<zh_sql_order_status>("belong_order", "step == 4");
        if (m_status)
        {
            m_time = m_status->timestamp;
        }
        if (vo->p_weight != 0)
        {
            p_weight_string = zh_double2string_reserve2(vo->p_weight) + "吨";
        }
        if (vo->m_weight != 0)
        {
            m_weight_string = zh_double2string_reserve2(vo->m_weight) + "吨";
            j_weight_string = zh_double2string_reserve2(abs(vo->p_weight - vo->m_weight));
        }
        content += "一次称重" + p_type + p_weight_string + "\n";
        content += "称重时间：" + p_time + "\n";
        content += "---------------\n";
        content += "二次称重" + m_type + m_weight_string + "\n";
        content += "称重时间：" + m_time + "\n";
        content += "---------------\n";
        content += "净重：" + j_weight_string + "吨\n";
        content += "---------------\n";
        content += "运送货物：" + vo->stuff_name + "\n";
        content += "派车公司：" + vo->company_name + "\n";
        qr_code = "http://" + std::string(getenv("BASE_URL"));
        qr_code += std::string(getenv("URL_REMOTE")) + "/#/mobile/field_opt/" + vo->order_number;
    }
    else
    {

        content += "---------------\n";
        content += "称重车辆：" + bound_vehicle_number + "\n";
        content += "---------------\n";
        content += "称重：" + zh_double2string_reserve2(fin_weight) + "吨\n";
        content += "称重时间：" + zh_rpc_util_get_timestring() + "\n";
        zh_sql_white_record tmp;
        tmp.date = zh_rpc_util_get_timestring();
        tmp.vehicle_number = bound_vehicle_number;
        tmp.weight = fin_weight;
        auto white_vehicle = sqlite_orm::search_record<zh_sql_vehicle>("main_vehicle_number == '%s' AND in_white_list != 0", bound_vehicle_number.c_str());
        if (white_vehicle)
        {
            tmp.use_stuff = white_vehicle->use_stuff;
            white_vehicle->use_stuff = "";
            white_vehicle->update_record();
        }
        tmp.insert_record();
    }
    std::string printer_ip;
    if (bound_scale.entry_config.cam_ip == enter_cam_ip)
    {
        printer_ip = bound_scale.exit_printer_ip;
    }
    else
    {
        printer_ip = bound_scale.entry_printer_ip;
    }
    system_management_handler::get_inst()->print_content(printer_ip, content, qr_code);
}

void scale_state_machine::set_traffic_lights_red()
{
    auto ip1 = bound_scale.traffic_light_ip1;
    auto ip2 = bound_scale.traffic_light_ip2;
    ZH_TRLI_set_red(ip1);
    ZH_TRLI_set_red(ip2);
}
void scale_state_machine::set_traffic_lights_green()
{
    auto ip1 = bound_scale.traffic_light_ip1;
    auto ip2 = bound_scale.traffic_light_ip2;
    ZH_TRLI_set_green(ip1);
    ZH_TRLI_set_green(ip2);
}
void scale_state_machine::open_trigger_switch()
{
    m_log.log("开启触发开关");
    trigger_switch = true;
    zh_hk_manual_trigger(last_enter_cam_ip);
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
            zh_hk_cast_exit_busy(bound_scale.entry_config.led_ip);
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
            zh_hk_cast_exit_busy(bound_scale.exit_config.led_ip);
        }
    }
}

void scale_state_machine::proc_trigger_qr(const std::string &_qr_code, const std::string &_road_ip)
{
    auto order_number_begin = _qr_code.find_last_of('/') + 1;
    auto order_number_end = _qr_code.find_last_of('\r');
    if (order_number_begin > _qr_code.length() || order_number_end > _qr_code.length())
    {
        return;
    }
    auto order_number = _qr_code.substr(order_number_begin, order_number_end - order_number_begin);
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", order_number.c_str());
    if (vo && vo->status >= 4)
    {
        print_weight_ticket(vo);
    }
    tdf_state_machine_lock a(*this);
    if (_road_ip == bound_scale.entry_qr_ip)
    {
        if (trigger_switch)
        {
            entry_param.qr_code = order_number;
        }
        else
        {
            zh_hk_cast_exit_busy(bound_scale.entry_config.led_ip);
        }
    }
    else if (_road_ip == bound_scale.exit_qr_ip)
    {
        if (trigger_switch)
        {
            exit_param.qr_code = order_number;
        }
        else
        {
            zh_hk_cast_exit_busy(bound_scale.exit_config.led_ip);
        }
    }
}
void scale_state_machine::proc_manual_confirm_scale()
{
    tdf_state_machine_lock a(*this);
    manual_confirm_scale = true;
    lack_weight = false;
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
            zh_hk_cast_exit_busy(bound_scale.entry_config.led_ip);
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
            zh_hk_cast_exit_busy(bound_scale.exit_config.led_ip);
        }
    }
}

void scale_state_machine::record_entry_exit()
{
    std::string tmp_vehicle = ctrl_policy.pass_permit(entry_param.vehicle_number, entry_param.id_no, entry_param.qr_code);
    if (!tmp_vehicle.empty())
    {
        bound_vehicle_number = tmp_vehicle;
        enter_cam_ip = bound_scale.entry_config.cam_ip;
        exit_cam_ip = bound_scale.exit_config.cam_ip;
        m_log.log("记录当前车辆:%s,入口：%s, 出口：%s", bound_vehicle_number.c_str(), enter_cam_ip.c_str(), exit_cam_ip.c_str());
        return;
    }

    tmp_vehicle = ctrl_policy.pass_permit(exit_param.vehicle_number, exit_param.id_no, exit_param.qr_code);
    if (!tmp_vehicle.empty())
    {
        bound_vehicle_number = tmp_vehicle;
        enter_cam_ip = bound_scale.exit_config.cam_ip;
        exit_cam_ip = bound_scale.entry_config.cam_ip;
        m_log.log("记录当前车辆:%s,入口：%s, 出口：%s", bound_vehicle_number.c_str(), enter_cam_ip.c_str(), exit_cam_ip.c_str());
        return;
    }
}

void scale_state_machine::broadcast_enter_scale()
{
    zh_hk_cast_holding(bound_scale.entry_config.led_ip, bound_vehicle_number);
    zh_hk_cast_holding(bound_scale.exit_config.led_ip, bound_vehicle_number);
}
void scale_state_machine::broadcast_leave_scale()
{
    bool permit_m_weight = false;
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", bound_vehicle_number.c_str());
    if (vo && vo->status == 3)
    {
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
        if (stuff && stuff->need_manual_scale)
        {
            if ((stuff->max_limit >= fin_weight && stuff->min_limit <= fin_weight) || manual_confirm_scale)
            {
                permit_m_weight = true;
            }
        }
    }
    else
    {
        permit_m_weight = true;
    }
    if (permit_m_weight)
    {
        zh_hk_cast_exit_scale(bound_scale.entry_config.led_ip, zh_double2string_reserve2(fin_weight), bound_vehicle_number);
        zh_hk_cast_exit_scale(bound_scale.exit_config.led_ip, zh_double2string_reserve2(fin_weight), bound_vehicle_number);
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
    ssm.set_traffic_lights_green();
}
void scale_sm_vehicle_come::before_leave(tdf_state_machine &_sm)
{
    auto &ssm = dynamic_cast<scale_state_machine &>(_sm);
    ssm.trigger_switch = false;
    ssm.set_traffic_lights_red();
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
        ssm.fin_weight = [&]() -> double
        {
            double ret = 0;
            for (auto &itr : ssm.continue_weight)
            {
                ret += itr;
            }
            if (ssm.continue_weight.size() > 0)
            {
                ret /= ssm.continue_weight.size();
            }
            return ret;
        }();
        ssm.fin_weight = floor(ssm.fin_weight * 100 + 0.5) / 100;
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
    auto vo = ssm.record_order();
    ssm.print_weight_ticket(vo);
    ssm.broadcast_leave_scale();
    ssm.close_timer();
    ssm.open_exit();
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
    ssm.scale_zero();
    ssm.close_timer();
}
gate_state_machine::gate_state_machine(
    const std::string &_road_ip,
    const std::string &_id_reader_ip,
    const std::string &_qr_ip,
    bool _is_entry) : m_log("gate sm " + _road_ip),
                      road_ip(_road_ip), id_reader_ip(_id_reader_ip), qr_ip(_qr_ip), is_entry(_is_entry),
                      id_api(_id_reader_ip, ZH_ID_READER_PORT,
                             [this](const std::string &_id)
                             {
                                 system_management_handler::get_inst()->id_result[this->id_reader_ip] = _id;
                                 this->proc_trigger_id_no(_id);
                                 this->trigger_sm();
                             })
{
    this->ctrl_policy.is_gate = true;
    if (_road_ip.length() > 0)
    {
        zh_sub_callback_cfg tmp_cfg;
        tmp_cfg.pData = this;
        tmp_cfg.callback = [](const std::string &_plate_no, const std::string &_road_ip, void *_pdata)
        {
            system_management_handler::get_inst()->cam_result[_road_ip] = _plate_no;
            if (_plate_no.length() > 0)
            {
                auto pthis = (gate_state_machine *)_pdata;
                pthis->proc_trigger_vehicle_number(_plate_no);
                pthis->trigger_sm();
            }
        };
        zh_hk_subcribe_event(_road_ip, tmp_cfg);
    }
    if (_qr_ip.length() > 0)
    {
        zh_sub_callback_cfg tmp_cfg;
        tmp_cfg.pData = this;
        tmp_cfg.callback = [](const std::string &_qr_code, const std::string &_qr_ip, void *_pdata)
        {
            system_management_handler::get_inst()->qr_result[_qr_ip] = _qr_code;
            if (_qr_code.length() > 0)
            {
                auto pthis = (gate_state_machine *)_pdata;
                pthis->m_log.log("read qr code:%s", _qr_code.c_str());
                pthis->proc_trigger_qr_code(_qr_code);
                pthis->trigger_sm();
            }
        };
        zh_qr_subscribe(_qr_ip, tmp_cfg);
    }
    m_cur_state.reset(new gate_sm_vehicle_come());
    device_config dc;
    system_management_handler::get_inst()->internal_get_device_config(dc);
    for (auto &itr : dc.gate)
    {
        if (itr.entry_config.cam_ip == _road_ip)
        {
            zh_hk_cast_empty(itr.entry_config.led_ip);
            break;
        }
        if (itr.exit_config.cam_ip == _road_ip)
        {
            zh_hk_cast_empty(itr.exit_config.led_ip);
            break;
        }
    }
}
gate_state_machine::~gate_state_machine()
{
    if (id_reader_timer >= 0)
    {
        tdf_main::get_inst().stop_timer(id_reader_timer);
        id_reader_timer = -1;
    }
    zh_hk_unsubcribe_event(road_ip);
    zh_qr_unsubscribe(qr_ip);
}
void gate_state_machine::clean_bound_info()
{
    param.clear();
}
void gate_state_machine::open_door()
{
    m_log.log("开门");
    zh_hk_ctrl_gate(road_ip, zh_hk_gate_open);
}
void gate_state_machine::gate_cast_accept()
{
    m_log.log("允许通过提示：" + param.vehicle_number);
    device_config dc;
    system_management_handler::get_inst()->internal_get_device_config(dc);
    std::string led_ip;
    for (auto &itr : dc.gate)
    {
        if (road_ip == itr.entry_config.cam_ip)
        {
            led_ip = itr.entry_config.led_ip;
            break;
        }
        if (road_ip == itr.exit_config.cam_ip)
        {
            led_ip = itr.exit_config.led_ip;
            break;
        }
    }
    if (is_entry)
    {
        zh_hk_cast_welcome(led_ip, param.vehicle_number);
    }
    else
    {
        zh_hk_cast_leave_bye(led_ip, param.vehicle_number);
    }
}
void gate_state_machine::gate_cast_reject()
{
    m_log.log("不允许通过提示" + param.vehicle_number);
    device_config dc;
    system_management_handler::get_inst()->internal_get_device_config(dc);
    std::string led_ip;
    for (auto &itr : dc.gate)
    {
        if (road_ip == itr.entry_config.cam_ip)
        {
            led_ip = itr.entry_config.led_ip;
            break;
        }
        if (road_ip == itr.exit_config.cam_ip)
        {
            led_ip = itr.exit_config.led_ip;
            break;
        }
    }
    std::string cur_vehicle = ctrl_policy.pass_permit(param.vehicle_number, param.id_no, param.qr_code);
    if (cur_vehicle == "")
    {
        cur_vehicle = "当前车辆";
    }
    if (!is_entry)
    {
        if (leave_timeout)
        {
            leave_timeout = false;
            zh_hk_cast_leave_timeout(led_ip, cur_vehicle);
        }
        else
        {
            zh_hk_cast_cannot_leave(led_ip, cur_vehicle);
        }
    }
    else
    {
        auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", cur_vehicle.c_str());
        if (!vo)
        {
            zh_hk_cast_no_order(led_ip, cur_vehicle);
        }
        else if (!vo->m_called)
        {
            zh_hk_cast_no_call(led_ip, cur_vehicle);
        }
    }
}
bool gate_state_machine::should_open()
{
    bool ret = false;
    auto cur_vehicle = ctrl_policy.pass_permit(param.vehicle_number, param.id_no, param.qr_code);
    if (cur_vehicle.length() > 0)
    {
        auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", cur_vehicle.c_str());
        if (is_entry)
        {
            if (vo)
            {
                if (vo->status == 1 && vo->m_called)
                {
                    ret = true;
                }
            }
            else
            {
                ret = true;
            }
        }
        else
        {
            if (vo)
            {
                if (vo->m_permit)
                {
                    if (vo->p_weight == 0)
                    {
                        ret = true;
                    }
                    else if (vo->m_weight > 0)
                    {
                        if (valid_stay_limit(*vo))
                        {
                            ret = true;
                        }
                        else
                        {
                            leave_timeout = true;
                        }
                    }
                }
            }
            else
            {
                ret = true;
            }
        }
    }
    return ret;
}

void gate_state_machine::record_vehicle_pass()
{
    m_log.log("出入场落库");
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", ctrl_policy.pass_permit(param.vehicle_number, param.id_no, param.qr_code).c_str());
    if (vo && vo->status == 1 && is_entry)
    {
        device_config dc;
        system_management_handler::get_inst()->internal_get_device_config(dc);
        for (auto &itr : dc.gate)
        {
            if (itr.entry_config.cam_ip == this->road_ip)
            {
                vo->enter_nvr_ip = itr.entry_nvr_ip + ":" + std::to_string(itr.entry_channel);
                vo->enter_cam_time = zh_rpc_util_get_timestring();
                break;
            }
        }
        auto status = zh_sql_order_status::make_in_status();
        change_order_status(*vo, status);
    }
    if (!is_entry && vo)
    {
        device_config dc;
        system_management_handler::get_inst()->internal_get_device_config(dc);
        for (auto &itr : dc.gate)
        {
            if (itr.exit_config.cam_ip == this->road_ip)
            {
                vo->exit_nvr_ip = itr.exit_nvr_ip + ":" + std::to_string(itr.exit_channel);
                vo->exit_cam_time = zh_rpc_util_get_timestring();
                break;
            }
        }
        auto out_status = zh_sql_order_status::make_out_status();
        change_order_status(*vo, out_status);
        auto end_status = zh_sql_order_status::make_end_status();
        auto save_hook_1 = zh_order_save_hook([](zh_sql_vehicle_order &) -> bool
                                              { return true; },
                                              dup_one_order);
        change_order_status(*vo, end_status, save_hook_1);
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
void gate_state_machine::proc_trigger_qr_code(const std::string &_qr_code)
{
    auto order_number_begin = _qr_code.find_last_of('/') + 1;
    auto order_number_end = _qr_code.find_last_of('\r');
    if (order_number_begin > _qr_code.length() || order_number_end > _qr_code.length())
    {
        return;
    }
    param.qr_code = _qr_code.substr(order_number_begin, order_number_end - order_number_begin);
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

    if (_qr_code.empty() && _id_no.empty() && _vehicle_number.empty())
    {
        judge_permit = false;
    }

    if (judge_permit)
    {
        device_config dc;
        system_management_handler::get_inst()->internal_get_device_config(dc);
        if (dc.auto_order && _vehicle_number.length() > 0)
        {
            if (!sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number = '%s' AND status != 100", _vehicle_number.c_str()))
            {
                zh_sql_vehicle_order tmp;
                tmp.main_vehicle_number = _vehicle_number;
                tmp.m_registered = true;
                tmp.m_called = true;
                tmp.m_permit = true;
                tmp.insert_record();
                tmp.order_number = std::to_string(time(nullptr)) + std::to_string(tmp.get_pri_id());
                auto create_status = zh_sql_order_status::make_create_status();
                auto before_come_status = zh_sql_order_status::make_before_come_status();
                change_order_status(tmp, create_status);
                change_order_status(tmp, before_come_status);
            }
        }
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
            if ((_id_no.length() > 0 && _id_no == itr.driver_id) ||
                (_qr_code.length() > 0 && itr.order_number == _qr_code) ||
                (_vehicle_number.length() > 0 && itr.main_vehicle_number == _vehicle_number))
            {
                ret = itr.main_vehicle_number;
                break;
            }
        }
        if (ret.empty())
        {
            std::string qurey_cmd = "(driver_id = '%s' OR main_vehicle_number = '%s') AND in_white_list == 1 AND use_stuff != ''";
            if (is_gate)
            {
                qurey_cmd = "(driver_id = '%s' OR main_vehicle_number = '%s') AND in_white_list == 1";
            }
            auto vw = sqlite_orm::search_record<zh_sql_vehicle>(qurey_cmd.c_str(), _id_no.c_str(), _vehicle_number.c_str());
            if (vw)
            {
                ret = vw->main_vehicle_number;
            }
        }
    }

    return ret;
}

static void pri_confirm_self_order(zh_sql_user_info user, const int64_t order_id, const bool continue_order)
{
    auto order = user.get_children<zh_sql_driver_self_order>("belong_user", "PRI_ID == %ld", order_id);
    if (!order)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    std::vector<vehicle_order_info> tmp;
    vehicle_order_info one_info;
    one_info.company_name = user.name;
    one_info.driver_id = order->driver_id;
    one_info.driver_name = order->driver_name;
    one_info.driver_phone = order->driver_phone;
    one_info.main_vehicle_number = order->main_vehicle_number;
    one_info.stuff_name = order->stuff_name;
    one_info.max_count = 35;
    if (continue_order)
    {
        one_info.end_time = zh_rpc_util_get_datestring().substr(0, 10);
    }
    tmp.push_back(one_info);
    if (pri_create_order(tmp))
    {
        order->remove_record();
    }
}

bool vehicle_order_center_handler::create_driver_self_order(const driver_self_order &order)
{
    bool ret = false;

    auto exist_record = sqlite_orm::search_record<zh_sql_driver_self_order>(
        "driver_phone == '%s' OR main_vehicle_number == '%s' OR driver_id == '%s'", order.driver_phone.c_str(), order.main_vehicle_number.c_str(), order.driver_id.c_str());
    if (exist_record)
    {
        ZH_RETURN_MSG("信息已存在");
    }
    zh_sql_driver_self_order tmp;
    tmp.driver_id = order.driver_id;
    tmp.driver_name = order.driver_name;
    tmp.driver_phone = order.driver_phone;
    tmp.main_vehicle_number = order.main_vehicle_number;
    tmp.stuff_name = order.stuff_name;
    auto user = sqlite_orm::search_record<zh_sql_user_info>("name == '%s'", order.belong_user_name.c_str());
    if (user)
    {
        tmp.set_parent(*user, "belong_user");
    }

    ret = tmp.insert_record();
    if (ret && user)
    {
        auto company = user->get_parent<zh_sql_contract>("belong_contract");
        if (company && !company->is_sale)
        {
            pri_confirm_self_order(*user, tmp.get_pri_id(), false);
        }
    }

    return ret;
}
bool vehicle_order_center_handler::confirm_driver_self_order(const std::string &ssid, const int64_t order_id, const bool continue_order)
{
    auto user = zh_rpc_util_get_online_user(ssid);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    pri_confirm_self_order(*user, order_id, continue_order);
    return true;
}
bool vehicle_order_center_handler::cancel_driver_self_order(const std::string &ssid, const int64_t order_id)
{
    bool ret = false;

    if (ssid.length() > 0)
    {
        auto user = zh_rpc_util_get_online_user(ssid);
        if (!user)
        {
            ZH_RETURN_NO_PRAVILIGE();
        }
    }

    auto order = sqlite_orm::search_record<zh_sql_driver_self_order>(order_id);
    if (!order)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    order->remove_record();
    ret = true;
    return ret;
}
void vehicle_order_center_handler::get_all_self_order(std::vector<driver_self_order> &_return, const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto orders = user->get_all_children<zh_sql_driver_self_order>("belong_user");
    for (auto &itr : orders)
    {
        driver_self_order tmp;
        tmp.belong_user_name = user->name;
        tmp.driver_id = itr.driver_id;
        tmp.driver_name = itr.driver_name;
        tmp.driver_phone = itr.driver_phone;
        tmp.id = itr.get_pri_id();
        tmp.main_vehicle_number = itr.main_vehicle_number;
        tmp.stuff_name = itr.stuff_name;
        _return.push_back(tmp);
    }
}
void vehicle_order_center_handler::get_self_order_by_phone(driver_self_order &_return, const std::string &driver_phone)
{
    auto order = sqlite_orm::search_record<zh_sql_driver_self_order>("driver_phone == '%s'", driver_phone.c_str());
    if (!order)
    {
        ZH_RETURN_MSG("未找到自助派车单");
    }
    auto user = order->get_parent<zh_sql_user_info>("belong_user");
    if (user)
    {
        _return.belong_user_name = user->name;
        _return.driver_id = order->driver_id;
        _return.driver_name = order->driver_name;
        _return.driver_phone = order->driver_phone;
        _return.id = order->get_pri_id();
        _return.main_vehicle_number = order->main_vehicle_number;
        _return.stuff_name = order->stuff_name;
    }
}

bool vehicle_order_center_handler::record_order_source_dest(const int64_t order_id, const std::string &source_dest_name)
{
    bool ret = false;

    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("status != 100 AND PRI_ID == %ld", order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    vo->source_dest_name = source_dest_name;
    ret = vo->update_record();

    return ret;
}

void vehicle_order_center_handler::get_driver_opt_history(vehicle_order_info &_return, const std::string &driver_phone)
{
    auto latest_vo = sqlite_orm::search_record<zh_sql_vehicle_order>("driver_phone == '%s'", driver_phone.c_str());
    if (latest_vo)
    {
        vehicle_order_detail tmp;
        make_vehicle_detail_from_sql(tmp, *latest_vo);
        _return = tmp.basic_info;
    }
}

void vehicle_order_center_handler::get_white_record_info(std::vector<white_record_info> &_return, const std::string &ssid, const int64_t _begin_date, const int64_t _end_date)
{
    auto user = zh_rpc_util_get_online_user(ssid);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    long begin_id = 0;
    long end_id = 0;
    auto begin_date = _begin_date;
    auto end_date = _end_date;
    while (begin_id <= 0)
    {
        auto begin_date_string = zh_rpc_util_get_timestring(begin_date).substr(0, 10);
        auto first_record = sqlite_orm::search_record<zh_sql_white_record>("date LIKE '%s%%'", begin_date_string.c_str());
        if (first_record)
        {
            begin_id = first_record->get_pri_id();
        }
        else
        {
            begin_date += 3600 * 24;
        }
        if (begin_date_string == zh_rpc_util_get_timestring(end_date).substr(0, 10))
        {
            break;
        }
    }
    begin_date = _begin_date;
    while (end_id <= 0)
    {
        auto end_date_string = zh_rpc_util_get_timestring(end_date).substr(0, 10);
        auto last_record = sqlite_orm::search_record<zh_sql_white_record>("date LIKE '%s%%' ORDER BY PRI_ID DESC", end_date_string.c_str());
        if (last_record)
        {
            end_id = last_record->get_pri_id();
        }
        else
        {
            end_date -= 3600 * 24;
        }
        if (end_date_string == zh_rpc_util_get_timestring(begin_date).substr(0, 10))
        {
            break;
        }
    }
    auto record = sqlite_orm::search_record_all<zh_sql_white_record>("PRI_ID >= %ld AND PRI_ID <= %ld", begin_id, end_id);
    for (auto &itr : record)
    {
        white_record_info tmp;
        tmp.id = itr.get_pri_id();
        tmp.date = itr.date;
        tmp.vehicle_number = itr.vehicle_number;
        tmp.weight = zh_double2string_reserve2(itr.weight);
        tmp.stuff_name = itr.use_stuff;
        auto vmr = sqlite_orm::search_record<zh_sql_vehicle>("main_vehicle_number == '%s'", tmp.vehicle_number.c_str());
        if (vmr)
        {
            tmp.group_name = vmr->group_name;
        }
        _return.push_back(tmp);
    }
}

void vehicle_order_center_handler::driver_get_last_30_order_number(std::vector<vehicle_order_detail> &_return, const std::string &driver_phone)
{
    auto vos = sqlite_orm::search_record_all<zh_sql_vehicle_order>("driver_phone == '%s' AND status == 100 ORDER BY PRI_ID DESC LIMIT 30", driver_phone.c_str());
    for (auto &itr : vos)
    {
        vehicle_order_detail tmp;
        make_vehicle_detail_from_sql(tmp, itr);
        _return.push_back(tmp);
    }
}

void vehicle_order_center_handler::export_order_by_condition(std::vector<vehicle_order_detail> &_return, const std::string &ssid, const std::string &begin_date, const std::string &end_date, const std::string &company_name, const std::string &stuff_name)
{
    auto opt_user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_ORDER, true);
    if (!opt_user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_ORDER);
    }
    std::string detail_query = "PRI_ID != 0";
    auto contract = opt_user->get_parent<zh_sql_contract>("belong_contract");
    if (contract)
    {
        detail_query = "company_name == '" + contract->name + "'";
    }
    if (company_name.length() > 0)
    {
        detail_query += " AND company_name == '" + company_name + "'";
    }
    if (stuff_name.length() > 0)
    {
        detail_query += " AND stuff_name == '" + stuff_name + "'";
    }
    detail_query += " AND datetime(m_cam_time) >= datetime('" + begin_date + "') AND datetime(m_cam_time) <= datetime('" + end_date + "')";
    auto all_order = sqlite_orm::search_record_all<zh_sql_vehicle_order>("(%s) ORDER BY datetime(m_cam_time) DESC", detail_query.c_str());
    for (auto &itr : all_order)
    {
        vehicle_order_detail tmp;
        make_vehicle_detail_from_sql(tmp, itr);
        _return.push_back(tmp);
    }
}

void vehicle_order_center_handler::go_through_plugin_que(std::vector<std::string> &_return, const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto que_items = g_nc_req_pip.go_throw_que();
    for (auto &itr : que_items)
    {
        _return.push_back(itr);
    }
}
void vehicle_order_center_handler::cancel_plugin_que(const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    g_nc_req_pip.cancel_que();
}

bool vehicle_order_center_handler::record_white_vehicle_stuff(const std::string &vehicle_number, const std::string &stuff_name)
{
    bool ret = false;

    auto white_vehicle = sqlite_orm::search_record<zh_sql_vehicle>("in_white_list != 0 AND main_vehicle_number == '%s'", vehicle_number.c_str());
    if (!white_vehicle)
    {
        ZH_RETURN_NO_VEHICLE();
    }
    white_vehicle->use_stuff = stuff_name;
    white_vehicle->use_date = zh_rpc_util_get_datestring();
    ret = white_vehicle->update_record();

    return ret;
}

void vehicle_order_center_handler::get_white_vehicle_stuff(std::string &_return, const std::string &vehicle_number)
{
    auto white_vehicle = sqlite_orm::search_record<zh_sql_vehicle>("in_white_list != 0 AND main_vehicle_number == '%s'", vehicle_number.c_str());
    if (!white_vehicle)
    {
        ZH_RETURN_NO_VEHICLE();
    }
    _return = white_vehicle->use_stuff;
}