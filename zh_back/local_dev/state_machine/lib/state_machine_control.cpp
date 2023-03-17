#include "state_machine_control.h"
#include "../../../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../../../zh_pub/zh_rpc_base/gen_code/cpp/plugin_management.h"
#include <functional>
#include <thrift/transport/THttpClient.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
static neb::CJsonObject send_and_recv(const neb::CJsonObject &_req, const std::string &_sm_name, bool no_need_resp = false)
{
    neb::CJsonObject ret;
    std::string mq_name = "/lib_client" + std::to_string(getpid());
    auto fd = mq_open(mq_name.c_str(), O_RDWR | O_CREAT, 0666, nullptr);
    if (fd >= 0)
    {
        auto send_fd = mq_open(("/" + _sm_name).c_str(), O_WRONLY);
        if (send_fd >= 0)
        {
            auto req_msg = _req;
            req_msg.Add("device_name", mq_name.substr(1));
            mq_send(send_fd, req_msg.ToString().c_str(), req_msg.ToString().length(), 0);
            mq_close(send_fd);
            char buff[9600] = {0};
            unsigned int prio = 0;
            timespec timeout_value;
            timeval now;
            gettimeofday(&now, NULL);
            timeout_value.tv_sec = 30 + now.tv_sec;
            timeout_value.tv_nsec = 0 + now.tv_usec;
            if (!no_need_resp)
            {
                auto recv_len = mq_timedreceive(fd, buff, sizeof(buff), &prio, &timeout_value);
                if (recv_len > 0)
                {
                    ret.Parse(std::string(buff, recv_len));
                }
            }
        }
        else
        {
            std::string faile_reason = "failed to open " + _sm_name;
            perror(faile_reason.c_str());
        }
        mq_close(fd);
        mq_unlink(mq_name.c_str());
    }
    else
    {
        perror("failed to open self mq");
    }

    return ret;
}
std::list<sm_config_meta> sm_control_get_all_sm()
{
    std::list<sm_config_meta> ret;
    auto cur_config = zh_plugin_conf_get_config("/conf/cur_sm.json");
    for (auto i = 0; i < cur_config.GetArraySize(); i++)
    {
        sm_config_meta tmp;
        tmp.name = cur_config[i]("name");
        tmp.sm_type = cur_config[i]("sm_type");
        neb::CJsonObject req;
        req.Add("msg_type", LOCAL_DEV_REQ_CUR_STATUS);
        tmp.status_string = send_and_recv(req, tmp.name)(LOCAL_DEV_EVENT_DEVICE_CUR_SATUS_KEY);
        req.ReplaceAdd("msg_type", LOCAL_DEV_REQ_CUR_WEIGHT);
        auto weight_string = send_and_recv(req, tmp.name)(LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT_KEY);
        if (weight_string.length() > 0)
        {
            tmp.cur_weight = std::stod(weight_string);
        }
        req.ReplaceAdd("msg_type", LOCAL_DEV_REQ_GATE_IS_CLOSE);
        req.Add(LOCAL_DEV_REQ_GATE_IS_CLOSE_KEY, true, true);
        send_and_recv(req, tmp.name).Get(LOCAL_DEV_EVENT_GATE_IS_CLOSE_KEY, tmp.enter_gate_is_close);
        req.Delete(LOCAL_DEV_REQ_GATE_IS_CLOSE_KEY);
        req.Add(LOCAL_DEV_REQ_GATE_IS_CLOSE_KEY, false, false);
        send_and_recv(req, tmp.name).Get(LOCAL_DEV_EVENT_GATE_IS_CLOSE_KEY, tmp.exit_gate_is_close);
        ret.push_back(tmp);
    }

    return ret;
}

std::string sm_control_take_picture(const std::string _name, bool _is_enter)
{
    std::string ret;

    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_TAKE_PICTURE);
    req.Add(LOCAL_DEV_REQ_TAKE_PICTURE_KEY, _is_enter, _is_enter);
    auto resp = send_and_recv(req, _name);

    ret = resp(LOCAL_DEV_EVENT_TAKE_PICTURE_KEY);

    return ret;
}
std::string sm_control_get_record(const std::string _name, bool _is_enter, const std::string &_begin_date, const std::string &_end_date)
{
    std::string ret;

    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_TAKE_VIDEO);
    req.Add(LOCAL_DEV_REQ_TAKE_VIDEO_KEY1, _is_enter, _is_enter);
    req.Add(LOCAL_DEV_REQ_TAKE_VIDEO_KEY2, _begin_date);
    req.Add(LOCAL_DEV_REQ_TAKE_VIDEO_KEY3, _end_date);
    auto resp = send_and_recv(req, _name);

    ret = resp(LOCAL_DEV_EVENT_GET_VIDEO_RECORD_KEY);

    return ret;
}

void sm_control_open_gate(const std::string &_name, bool _is_enter, bool _is_open)
{
    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_OPEN_GATE);
    req.Add(LOCAL_DEV_REQ_OPEN_GATE_KEY1, _is_open, _is_open);
    req.Add(LOCAL_DEV_REQ_OPEN_GATE_KEY2, _is_enter, _is_enter);
    send_and_recv(req, _name, true);
}
void sm_control_trigger(const std::string &_name, bool _is_enter, const std::string &_plate)
{
    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_MANUAL_TIGGER);
    req.Add(LOCAL_DEV_REQ_MANUAL_TIGGER_KEY1, _plate);
    req.Add(LOCAL_DEV_REQ_MANUAL_TIGGER_KEY2, _is_enter, _is_enter);
    send_and_recv(req, _name, true);
}
void sm_control_reset(const std::string &_name)
{
    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_RESET);
    send_and_recv(req, _name, true);
}
void sm_control_confirm_weight(const std::string &_name)
{
    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_CONFIRM_WEIGHT);
    send_and_recv(req, _name, true);
}

void sm_control_led_cast(const std::string &_name, bool _is_enter, const std::string &_content)
{
    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_LED_CAST);
    req.Add(LOCAL_DEV_REQ_LED_CAST_KEY1, _is_enter, _is_enter);
    req.Add(LOCAL_DEV_REQ_LED_CAST_KEY2, _content);
    send_and_recv(req, _name, true);
}
void sm_control_print(const std::string &_name, bool _is_enter, const std::string &_content, const std::string &_qr_content)
{
    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_PRINT);
    req.Add(LOCAL_DEV_REQ_PRINT_KEY1, _is_enter, _is_enter);
    req.Add(LOCAL_DEV_REQ_PRINT_KEY2, _content);
    req.Add(LOCAL_DEV_REQ_PRINT_KEY3, _qr_content);
    send_and_recv(req, _name, true);
}
void sm_control_set_traffic_light(const std::string &_name, bool _is_enter, bool _is_green)
{
    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_TRFC_SET);
    req.Add(LOCAL_DEV_REQ_TRFC_SET_KEY1, _is_enter, _is_enter);
    req.Add(LOCAL_DEV_REQ_TRFC_SET_KEY2, _is_green, _is_green);
    send_and_recv(req, _name, true);
}

#define THR_DEF_CIENT(x) x##Client *client = nullptr
#define THR_CONNECT(x) std::shared_ptr<TTransport> transport(new THttpClient("localhost", 8123, "/zh_rpc"));std::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport)); transport->open();  std::shared_ptr<TMultiplexedProtocol> mp(new TMultiplexedProtocol(protocol, #x)); client = new x##Client(mp)
#define TRH_CLOSE() transport->close()
static void generate_order_event(zh_sql_vehicle_order &_order)
{
    using namespace ::apache::thrift;
    using namespace ::apache::thrift::protocol;
    using namespace ::apache::thrift::transport;
    THR_DEF_CIENT(plugin_management);
    THR_CONNECT(plugin_management);
    client->ext_deliver_event(_order.order_number, _order.status);
    TRH_CLOSE();
}
bool change_order_status(zh_sql_vehicle_order &_order, zh_sql_order_status &_status, const zh_order_save_hook &_hook)
{
    tdf_log tmp_log("scale sm");
    long begin_change = time(nullptr);
    _order.push_status(_status, _hook);
    long change_status_point = time(nullptr);
    tmp_log.log("change spend:%ld", change_status_point - begin_change);
    generate_order_event(_order);
    long gen_event_point = time(nullptr);
    tmp_log.log("gen event spend:%ld", gen_event_point - change_status_point);
    auto voc = vehicle_order_center_handler::get_inst();
    if (voc)
    {
        voc->execute_auto_call(_order.stuff_name);
    }
    long auto_call_point = time(nullptr);
    tmp_log.log("exe auto call:%ld", auto_call_point - gen_event_point);
    return true;
}
void recalcu_balance_inventory(zh_sql_vehicle_order &_vo, const std::string &_ssid)
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
    auto sii = sqlite_orm::search_record<zh_sql_stuff_inv_info>("name == '%s'", _vo.bound_inv_name.c_str());
    if (sii)
    {
        auto sie = sii->get_children<zh_sql_stuff_inv_element>("belong_sii", "stuff_name == '%s'", _vo.stuff_name.c_str());
        if (sie)
        {
            sie->inventory += _vo.p_weight - _vo.m_weight;
            sie->update_record();
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
void dup_one_order(zh_sql_vehicle_order &vo)
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
            try
            {

                pri_create_order(tmp, false);
            }
            catch (...)
            {
            }
        }
    }
}
static std::string _is_in_black_list(const vehicle_order_info &_order)
{
    std::string ret;
    std::string tail_string = "在黑名单";

    auto all_black_info = sqlite_orm::search_record_all<zh_sql_vehicle>("in_black_list == 1");
    for (auto &itr : all_black_info)
    {
        if (_order.main_vehicle_number == itr.main_vehicle_number)
        {
            ret = _order.main_vehicle_number + tail_string;
            break;
        }
        if (_order.behind_vehicle_number == itr.behind_vehicle_number && _order.behind_vehicle_number.size() > 0)
        {
            ret = _order.behind_vehicle_number + tail_string;
            break;
        }
        if (_order.driver_id == itr.driver_id)
        {
            ret = _order.driver_id + tail_string;
            break;
        }
    }

    return ret;
}

static bool vehicle_order_is_dup(const vehicle_order_info &order)
{
    bool ret = false;

    auto exist_record = sqlite_orm::search_record<zh_sql_vehicle_order>("PRI_ID != %ld AND status != 100 AND (main_vehicle_number == '%s' OR (driver_id != '' AND driver_id == '%s') OR (driver_phone != '' AND driver_phone == '%s'))", order.id, order.main_vehicle_number.c_str(), order.driver_id.c_str(), order.driver_phone.c_str());
    if (exist_record)
    {
        ret = true;
    }

    return ret;
}
bool pri_create_order(const std::vector<vehicle_order_info> &orders, bool from_api, const std::string &ssid)
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

    auto order_is_same = [](const vehicle_order_info &_fir, const vehicle_order_info &_sec) -> bool
    {
        bool same = false;
        if (_fir.main_vehicle_number == _sec.main_vehicle_number)
        {
            same = true;
        }
        else if (_fir.behind_vehicle_number.length() > 0 && _fir.behind_vehicle_number == _sec.behind_vehicle_number)
        {
            same = true;
        }
        else if (_fir.driver_phone.length() > 0 && _fir.driver_phone == _sec.driver_phone)
        {
            same = true;
        }
        else if (_fir.driver_id.length() > 0 && _fir.driver_id == _sec.driver_id)
        {
            same = true;
        }

        return same;
    };

    for (auto itr = orders.begin(); itr != orders.end(); ++itr)
    {
        auto find_begin_ret = std::find_if(
            orders.begin(), itr,
            [&](const vehicle_order_info &_val)
            {
                return order_is_same(*itr, _val);
            });
        if (find_begin_ret != itr)
        {
            ZH_RETURN_DUP_ORDER();
        }
        auto find_end_ret = std::find_if(
            itr + 1, orders.end(),
            [&](const vehicle_order_info &_val)
            {
                return order_is_same(*itr, _val);
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
        auto black_info = _is_in_black_list(order);
        if (black_info.length() > 0)
        {
            ZH_RETURN_MSG(black_info);
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

        tmp.from_api = from_api;

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
            auto enable_check_in = system_management_handler::get_inst()->check_in_enabled();
            if (!enable_check_in)
            {
                tmp.m_registered = true;
                tmp.check_in_timestamp = time(nullptr);
            }
            ret = tmp.update_record();
        }
    }

    return ret;
}
std::string order_ticket_content(zh_sql_vehicle_order &_order)
    {
        std::string content;
        if (_order.bl_number.length() > 0)
        {
            content += "磅单号：" + _order.bl_number + "\n";
        }
        content += "称重车辆：" + _order.main_vehicle_number + "\n";
        std::string m_weight_string = "未称重";
        std::string j_weight_string = "未知";
        std::string p_weight_string = m_weight_string;
        std::string p_time;
        std::string m_time;
        std::string p_type = "（毛重）：";
        std::string m_type = "（皮重）：";
        auto company = sqlite_orm::search_record<zh_sql_contract>("name == '%s'", _order.company_name.c_str());
        if (company && company->is_sale)
        {
            auto tmp_type = p_type;
            p_type = m_type;
            m_type = tmp_type;
        }
        p_time = _order.p_cam_time;
        m_time = _order.m_cam_time;
        std::string max_load = _order.max_load;
        if (_order.p_weight != 0)
        {
            p_weight_string = zh_double2string_reserve2(_order.p_weight) + "吨";
        }
        if (_order.m_weight != 0)
        {
            m_weight_string = zh_double2string_reserve2(_order.m_weight) + "吨";
            j_weight_string = zh_double2string_reserve2(abs(_order.p_weight - _order.m_weight));
            max_load = "";
        }
        content += "一次称重" + p_type + p_weight_string + "\n";
        content += "称重时间：" + p_time + "\n";
        content += "二次称重" + m_type + m_weight_string + "\n";
        content += "称重时间：" + m_time + "\n";
        content += "净重：" + j_weight_string + "吨\n";
        content += "运送货物：" + _order.stuff_name + "\n";
        content += "派车公司：" + _order.company_name + "\n";
        if (max_load.length() > 0)
        {
            content += "最大装车量：" + max_load + "\n吨";
        }
        return content;
    }