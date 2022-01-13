#include "vehicle_order_center_imp.h"
#include "zh_rpc_util.h"
#include "../zh_hk_gate/lib/zh_hk_gate.h"
#include "../zh_id_reader/lib/zh_id_reader.h"
#include "system_management_imp.h"
#include "../zh_raster/lib/zh_raster.h"
#include "../zh_scale/lib/zh_scale.h"
#include "../zh_qr/lib/zh_qr_lib.h"
#include <sstream>

vehicle_order_center_handler *vehicle_order_center_handler::m_inst = nullptr;
std::map<std::string, std::shared_ptr<scale_state_machine>> vehicle_order_center_handler::ssm_map;
std::map<std::string, std::shared_ptr<gate_state_machine>> vehicle_order_center_handler::gsm_map;
void vehicle_order_center_handler::get_order_by_anchor(std::vector<vehicle_order_info> &_return, const std::string &ssid, const int64_t anchor, const std::string &status_name)
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

    auto orders = sqlite_orm::search_record_all<zh_sql_vehicle_order>("(%s) ORDER BY PRI_ID DESC LIMIT 30 OFFSET %ld", detail_query.c_str(), anchor);
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
        auto attachment = itr.get_parent<zh_sql_file>("attachment");
        if (attachment)
        {
            tmp.attachment = attachment->name;
        }
        auto enter_weight_attachment = itr.get_parent<zh_sql_file>("enter_weight_attachment");
        if (enter_weight_attachment)
        {
            tmp.enter_weight_attachment = enter_weight_attachment->name;
        }
        tmp.enter_weight = itr.enter_weight;
        tmp.need_enter_weight = itr.need_enter_weight;

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

    auto exist_record = sqlite_orm::search_record<zh_sql_vehicle_order>("PRI_ID != %ld AND status != 100 AND (main_vehicle_number == '%s' OR driver_id == '%s')", order.id, order.main_vehicle_number.c_str(), order.driver_id.c_str());
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
        tmp.driver_name = order.driver_name;
        tmp.driver_phone = order.driver_phone;
        tmp.main_vehicle_number = order.main_vehicle_number;
        tmp.stuff_name = order.stuff_name;
        tmp.company_name = order.company_name;
        tmp.status = -1;
        auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", order.stuff_name.c_str());
        if (stuff && stuff->need_enter_weight)
        {
            tmp.need_enter_weight = 1;
        }

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

void make_vehicle_detail_from_sql(vehicle_order_detail &_return, zh_sql_vehicle_order &_order)
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
    _return.basic_info = tmp;
    _return.confirmed = vo->m_permit;
    _return.has_called = vo->m_called;
    _return.registered = vo->m_registered;
}

void vehicle_order_center_handler::get_order_detail(vehicle_order_detail &_return, const std::string &ssid, const std::string &order_number)
{
    auto user = zh_rpc_util_get_online_user(ssid, 3);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
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
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s'", order_number.c_str());
    if (!vo || vo->status >= 4)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    if (vo->p_weight == 0)
    {
        ZH_RETURN_MSG("未完成一次称重，不允许确认");
    }
    vo->m_permit = confirmed;
    ret = vo->update_record();
    return ret;
}

bool vehicle_order_center_handler::update_vehicle_order(const std::string &ssid, const vehicle_order_info &order)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, 1);
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
bool vehicle_order_center_handler::driver_check_in(const int64_t order_id, const bool is_cancel)
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
    vo->m_registered = is_cancel ? 0 : 1;
    if (!vo->m_registered)
    {
        vo->m_called = 0;
    }
    ret = vo->update_record();

    return ret;
}

void vehicle_order_center_handler::driver_get_order(vehicle_order_detail &_return, const std::string &order_number)
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("order_number == '%s' AND status != 100 AND status != 0", order_number.c_str());
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    make_vehicle_detail_from_sql(_return, *vo);
}

bool vehicle_order_center_handler::call_vehicle(const std::string &ssid, const int64_t order_id, const bool is_cancel)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
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
    ret = vo->update_record();
    if (ret && vo->m_called && orig_called != vo->m_called)
    {
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

void vehicle_order_center_handler::get_registered_vehicle(std::vector<vehicle_order_detail> &_return, const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, 2);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto vos = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status >= 1 AND status != 100 AND m_registered == 1");
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
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }

    auto status = zh_sql_order_status::make_p_status(ssid);
    vo->push_status(status);
    vo->p_weight = weight;
    ret = vo->update_record();

    return ret;
}
bool vehicle_order_center_handler::manual_set_m_weight(const std::string &ssid, const int64_t order_id, const double weight)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }

    auto status = zh_sql_order_status::make_m_status(ssid);
    vo->push_status(status);
    vo->m_weight = weight;
    ret = vo->update_record();

    return ret;
}

bool vehicle_order_center_handler::manual_close(const std::string &ssid, const int64_t order_id)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, 1);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>(order_id);
    if (!vo)
    {
        ZH_RETURN_NO_ORDER();
    }
    auto status = zh_sql_order_status::make_end_status(ssid);
    vo->push_status(status);
    auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
    if (stuff)
    {
        stuff->inventory += vo->p_weight - vo->m_weight;
        stuff->update_record(ssid);
    }

    ret = true;

    return ret;
}

void vehicle_order_center_handler::get_order_statistics(vehicle_order_statistics &_return, const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, 2);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto cvo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 0");
    auto convo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 1");
    auto evo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 2");
    auto fwvo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 3");
    auto swvo = sqlite_orm::search_record_all<zh_sql_vehicle_order>("status == 4");
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

    auto opt_user = zh_rpc_util_get_online_user(ssid, 1);
    if (!opt_user)
    {
        ZH_RETURN_NO_PRAVILIGE();
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

scale_state_machine::scale_state_machine(const device_scale_config &_config) : m_log(_config.name + " scale sm"), bound_scale(_config)
{
    auto id_read_call_back_entry = [](void *_private)
    {
        auto ssm = (scale_state_machine *)_private;
        auto id_ret = zh_read_id_no(ssm->bound_scale.entry_id_reader_ip, ZH_ID_READER_PORT);
        system_management_handler::get_inst()->id_result[ssm->bound_scale.entry_id_reader_ip] = id_ret;
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
        system_management_handler::get_inst()->id_result[ssm->bound_scale.exit_id_reader_ip] = id_ret;
        if (id_ret.length() > 0)
        {
            ssm->proc_trigger_id_read(id_ret, ssm->bound_scale.exit_id_reader_ip);
            ssm->trigger_sm();
        }
    };
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
void scale_state_machine::open_scale_timer()
{
    m_log.log("开启定时器");
    timer_fd = tdf_main::get_inst().start_timer(
        1,
        [](void *_private)
        {
            auto ssm = (scale_state_machine *)_private;
            if (!raster_was_block(ssm->bound_scale.raster_ip[0], ZH_RASTER_PORT) && !raster_was_block(ssm->bound_scale.raster_ip[1], ZH_RASTER_PORT))
            {
                auto scale_ret = get_current_weight(ssm->bound_scale.scale_ip, ZH_SCALE_PORT, ssm->bound_scale.scale_brand);
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
                if (ava != 0 && p_dev / ava > 0.01)
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
    m_log.log("关闭定时器");
    if (timer_fd >= 0)
    {
        tdf_main::get_inst().stop_timer(timer_fd);
        timer_fd = -1;
        continue_weight.clear();
        fin_weight = 0;
    }
}
void scale_state_machine::clean_bound_info()
{
    m_log.log("清理绑定数据");
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
            if (vo->p_weight == 0 && vo->m_called)
            {
                ret = true;
            }
            else if (vo->m_weight == 0 && vo->m_permit && vo->m_called)
            {
                ret = true;
            }
        }
        else
        {
            ret = true;
        }
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

std::unique_ptr<zh_sql_vehicle_order> scale_state_machine::record_order()
{
    auto vo = sqlite_orm::search_record<zh_sql_vehicle_order>("main_vehicle_number == '%s' AND status != 100", bound_vehicle_number.c_str());
    if (vo)
    {
        if (vo->status < 3)
        {
            auto status = zh_sql_order_status::make_p_status();
            vo->p_weight = fin_weight;
            vo->push_status(status);
        }
        else if (vo->status < 4)
        {
            auto status = zh_sql_order_status::make_m_status();
            vo->m_weight = fin_weight;
            vo->push_status(status);
            auto stuff = sqlite_orm::search_record<zh_sql_stuff>("name == '%s'", vo->stuff_name.c_str());
            if (stuff)
            {
                stuff->inventory = vo->p_weight - vo->m_weight;
                stuff->update_record(vo->order_number);
            }
            device_config dc;
            system_management_handler::get_inst()->internal_get_device_config(dc);
            if (dc.gate.empty())
            {
                auto end_status = zh_sql_order_status::make_end_status();
                vo->push_status(end_status);
            }
        }
    }
    return vo;
}
void scale_state_machine::print_weight_ticket(const std::unique_ptr<zh_sql_vehicle_order> &vo)
{
    auto double2string_reserve2 = [](double _value) -> std::string
    {
        std::stringstream ss;
        ss.setf(std::ios::fixed);
        ss.precision(2);
        ss << _value;
        return ss.str();
    };
    std::string content;
    content += "---------------\n";
    content += "称重车辆：" + bound_vehicle_number + "\n";
    content += "---------------\n";
    std::string qr_code;
    if (vo)
    {
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
            p_weight_string = double2string_reserve2(vo->p_weight) + "吨";
        }
        if (vo->m_weight != 0)
        {
            m_weight_string = double2string_reserve2(vo->m_weight) + "吨";
            j_weight_string = double2string_reserve2(abs(vo->p_weight - vo->m_weight));
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
        qr_code = getenv("BASE_URL");
        qr_code += ".d8sis.cn/#/field_opt/" + vo->order_number;
    }
    else
    {
        content += "称重：" + double2string_reserve2(fin_weight) + "吨\n";
        content += "称重时间：" + zh_rpc_util_get_timestring() + "\n";
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
void scale_state_machine::open_trigger_switch()
{
    m_log.log("开启触发开关");
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
    std::string hold_led_ip;
    if (bound_scale.entry_config.cam_ip == enter_cam_ip)
    {
        hold_led_ip = bound_scale.exit_config.led_ip;
    }
    else if (bound_scale.exit_config.cam_ip == enter_cam_ip)
    {
        hold_led_ip = bound_scale.entry_config.led_ip;
    }
    if (hold_led_ip.length() > 0)
    {
        zh_hk_cast_holding(hold_led_ip);
    }
}
void scale_state_machine::broadcast_leave_scale()
{
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
        zh_hk_cast_exit_scale(led_ip);
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
        ssm.fin_weight = [&]() -> double
        {
            double ret = 0;
            for (auto &itr : ssm.continue_weight)
            {
                ret += itr;
            }
            ret /= ssm.continue_weight.size();
            return ret;
        }();
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
gate_state_machine::gate_state_machine(
    const std::string &_road_ip,
    const std::string &_id_reader_ip,
    const std::string &_qr_ip,
    bool _is_entry) : m_log("gate sm " + _road_ip),
                      road_ip(_road_ip), id_reader_ip(_id_reader_ip), qr_ip(_qr_ip), is_entry(_is_entry)
{
    if (_id_reader_ip.length() > 0)
    {
        id_reader_timer = tdf_main::get_inst().start_timer(
            1,
            [](void *_private)
            {
                auto gsm = (gate_state_machine *)_private;
                auto id_no = zh_read_id_no(gsm->id_reader_ip, ZH_ID_READER_PORT);
                system_management_handler::get_inst()->id_result[gsm->id_reader_ip] = id_no;
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
        zh_hk_cast_cannot_leave(led_ip, cur_vehicle);
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
                if (vo->status == 4)
                {
                    ret = true;
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
        auto status = zh_sql_order_status::make_in_status();
        vo->push_status(status);
    }
    if (!is_entry && vo)
    {
        auto status = zh_sql_order_status::make_end_status();
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
        if (ret.empty())
        {
            auto vw = sqlite_orm::search_record<zh_sql_vehicle>("(driver_id = '%s' OR main_vehicle_number = '%s') AND in_white_list == 1", _id_no.c_str(), _vehicle_number.c_str());
            if (vw)
            {
                ret = vw->main_vehicle_number;
            }
        }
    }

    return ret;
}