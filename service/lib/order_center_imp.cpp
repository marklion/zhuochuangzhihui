#include "order_center_imp.h"
#include "rpc_include.h"
#include <iomanip> // 包含 setw 和 setfill
#define CHECK_DUP_ITEM(x, y)                           \
    do                                                 \
    {                                                  \
        if (ret.length() <= 0 && order.x.length() > 0) \
        {                                              \
            order_search_cond tmp;                     \
            tmp.exp_status = 100;                      \
            tmp.x = order.x;                           \
            std::vector<vehicle_order_info> es;        \
            search_order(es, tmp);                     \
            if (es.size() > 0)                         \
            {                                          \
                ret = y + order.x + "已存在";          \
            }                                          \
        }                                              \
    } while (0)
std::string order_center_handler::order_is_dup(const vehicle_order_info &order)
{
    std::string ret;

    CHECK_DUP_ITEM(driver_phone, "司机电话");
    CHECK_DUP_ITEM(driver_id, "司机身份证");
    CHECK_DUP_ITEM(plate_number, "车牌号");

    return ret;
}

std::unique_ptr<sql_order> order_center_handler::get_order_by_number(const std::string &_order_number)
{
    auto es = sqlite_orm::search_record<sql_order>("order_number == '%s'", _order_number.c_str());
    if (es)
    {
        return es;
    }
    return std::unique_ptr<sql_order>();
}

void order_center_handler::auto_call_next()
{
    running_rule tmp;
    THR_CALL_BEGIN(config_management);
    client->get_rule(tmp);
    THR_CALL_END();
    if (tmp.auto_call_count > 0)
    {
        auto called_orders = sqlite_orm::search_record_all<sql_order>("status != 100 AND reg_info_time != '' AND call_info_time != '' ORDER BY datetime(reg_info_time)");
        if (called_orders.size() < tmp.auto_call_count)
        {
            auto wait_order = sqlite_orm::search_record<sql_order>("status == 1 AND reg_info_time != '' AND call_info_time == '' ORDER BY datetime(reg_info_time)");
            if (wait_order)
            {
                order_call(wait_order->order_number, true, "(自动)");
                auto_call_next();
            }
        }
    }
}

void order_center_handler::push_zyzl(const std::string &_order_number)
{
    running_rule rule;
    THR_CALL_BEGIN(config_management);
    client->get_rule(rule);
    THR_CALL_END();
    auto vo = get_order_by_number(_order_number);
    if (vo && vo->status != 100 &&
        vo->p_weight != 0 && vo->m_weight != 0 &&
        rule.zyzl_ssid.length() != 0 && rule.zyzl_host.length() != 0)
    {
        auto tmp = zyzl_plugin::get_inst();
        auto ticket_no = gen_ticket_no();
        if (ticket_no.empty())
        {
            ticket_no = _order_number;
        }
        tmp->push_weight(vo->plate_number, vo->p_time, vo->m_time, vo->p_weight, vo->m_weight, std::abs(vo->p_weight - vo->m_weight), ticket_no, vo->seal_no);
    }
}

std::string order_center_handler::gen_ticket_no()
{
    std::string ret;
    running_rule rule;
    THR_CALL_BEGIN(config_management);
    client->get_rule(rule);
    THR_CALL_END();
    if (rule.date_ticket_prefix.length() > 0)
    {
        ret = rule.date_ticket_prefix;
        auto to_tic = sqlite_orm::search_record<sql_ticket_today_index>(1);
        if (!to_tic)
        {
            sql_ticket_today_index tmp;
            tmp.today_index = 0;
            tmp.today_date = util_get_datestring();
            tmp.insert_record();
        }
        to_tic.reset(sqlite_orm::search_record<sql_ticket_today_index>(1).release());
        if (to_tic->today_date != util_get_datestring())
        {
            to_tic->today_index = 0;
            to_tic->today_date = util_get_datestring();
        }
        to_tic->today_index++;
        to_tic->update_record();

        auto date_string = util_get_datestring();
        date_string = util_join_string(util_split_string(date_string, "-"), "");
        ret += date_string;
        auto str = std::to_string(to_tic->today_index);
        ret += std::string(4 - str.length(), '0') + str;
    }

    return ret;
}

long order_center_handler::gen_reg_no()
{
    long ret = 0;
    auto to_tic = sqlite_orm::search_record<sql_reg_no_today_index>(1);
    if (!to_tic)
    {
        sql_reg_no_today_index tmp;
        tmp.today_index = 0;
        tmp.today_date = util_get_datestring();
        tmp.insert_record();
    }
    to_tic.reset(sqlite_orm::search_record<sql_reg_no_today_index>(1).release());
    if (to_tic->today_date != util_get_datestring())
    {
        to_tic->today_index = 0;
        to_tic->today_date = util_get_datestring();
    }
    to_tic->today_index++;
    to_tic->update_record();
    ret = to_tic->today_index;
    return ret;
}

void order_center_handler::close_order(sql_order &_order)
{
    push_zyzl(_order.order_number);
    _order.status = 100;
    _order.update_record();
    auto_call_next();
}

void order_center_handler::db_2_rpc(sql_order &_db, vehicle_order_info &_rpc)
{
    _rpc.back_plate_number = _db.back_plate_number;
    _rpc.call_info.operator_name = _db.call_info_name;
    _rpc.call_info.operator_time = _db.call_info_time;
    _rpc.company_name = _db.company_name;
    _rpc.confirm_info.operator_name = _db.confirm_info_name;
    _rpc.confirm_info.operator_time = _db.confirm_info_time;
    _rpc.driver_id = _db.driver_id;
    _rpc.driver_name = _db.driver_name;
    _rpc.driver_phone = _db.driver_phone;
    _rpc.enter_weight = _db.enter_weight;
    _rpc.id = _db.get_pri_id();
    _rpc.is_sale = _db.is_sale;
    _rpc.m_time = _db.m_time;
    _rpc.m_weight = _db.m_weight;
    _rpc.order_number = _db.order_number;
    _rpc.p_time = _db.p_time;
    _rpc.p_weight = _db.p_weight;
    _rpc.plate_number = _db.plate_number;
    _rpc.reg_info.operator_name = _db.reg_info_name;
    _rpc.reg_info.operator_time = _db.reg_info_time;
    _rpc.seal_no = _db.seal_no;
    _rpc.status = _db.status;
    _rpc.stuff_from = _db.stuff_from;
    _rpc.stuff_name = _db.stuff_name;
    _rpc.reg_no = _db.reg_no;
    auto attchs = _db.get_all_children<sql_order_attach>("belong_order");
    auto hns = _db.get_all_children<sql_order_history>("belong_order");
    for (auto &itr : attchs)
    {
        vehicle_order_attachment tmp;
        db_2_rpc(itr, tmp);
        _rpc.order_attachs.push_back(tmp);
    }
    for (auto &itr : hns)
    {
        vehicle_order_history_node tmp;
        db_2_rpc(itr, tmp);
        _rpc.history_records.push_back(tmp);
    }
}

void order_center_handler::db_2_rpc(sql_order_attach &_db, vehicle_order_attachment &_rpc)
{
    _rpc.att_name = _db.att_name;
    _rpc.att_path = _db.att_path;
    _rpc.id = _db.get_pri_id();
}

void order_center_handler::db_2_rpc(sql_order_history &_db, vehicle_order_history_node &_rpc)
{
    _rpc.id = _db.get_pri_id();
    _rpc.node_caller = _db.node_caller;
    _rpc.node_name = _db.node_name;
    _rpc.occour_time = _db.occour_time;
}

void order_center_handler::rpc_2_db(const vehicle_order_info &_rpc, sql_order &_db)
{
    _db.back_plate_number = _rpc.back_plate_number;
    _db.company_name = _rpc.company_name;
    _db.driver_id = _rpc.driver_id;
    _db.driver_name = _rpc.driver_name;
    _db.driver_phone = _rpc.driver_phone;
    _db.enter_weight = _rpc.enter_weight;
    _db.is_sale = _rpc.is_sale;
    _db.plate_number = _rpc.plate_number;
    _db.stuff_from = _rpc.stuff_from;
    _db.stuff_name = _rpc.stuff_name;
    auto id_has_x = _db.driver_id;
    auto x_index = id_has_x.find_last_of('X');
    if (x_index != std::string::npos)
    {
        id_has_x[x_index] = 'x';
    }
    _db.driver_id = id_has_x;
}

bool order_center_handler::add_order(const vehicle_order_info &order)
{
    bool ret = false;

    auto dup_ret = order_is_dup(order);
    if (dup_ret.length() > 0)
    {
        ZH_RETURN_MSG(dup_ret);
    }

    sql_order tmp;
    rpc_2_db(order, tmp);
    tmp.status = 1;
    if (tmp.insert_record())
    {
        tmp.order_number = std::to_string(time(nullptr)) + std::to_string(tmp.get_pri_id());
        ret = tmp.update_record();
    }

    return ret;
}

bool order_center_handler::del_order(const std::string &order_number)
{
    bool ret = false;

    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }

    if (es->status == 2)
    {
        ZH_RETURN_MSG("正在执行，无法关闭");
    }

    close_order(*es);

    return true;
}

bool order_center_handler::update_order(const vehicle_order_info &order)
{
    bool ret = false;

    auto es = get_order_by_number(order.order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }
    rpc_2_db(order, *es);
    ret = es->update_record();

    return ret;
}

#define SEARCH_COND_APPEND(x)                 \
    if (cond.x.length() > 0)                  \
    {                                         \
        query_cond += " AND ";                \
        query_cond += #x;                     \
        query_cond += " == '" + cond.x + "'"; \
    }

void order_center_handler::search_order(std::vector<vehicle_order_info> &_return, const order_search_cond &cond)
{
    std::string query_cond = "PRI_ID != 0";

    if (cond.begin_time.length() > 0)
    {
        query_cond += " AND datetime(p_time) >= datetime('" + cond.begin_time + "')";
    }
    if (cond.end_time.length() > 0)
    {
        query_cond += " AND datetime(p_time) <= datetime('" + cond.end_time + "')";
    }
    SEARCH_COND_APPEND(company_name);
    SEARCH_COND_APPEND(stuff_name);
    SEARCH_COND_APPEND(driver_id);
    SEARCH_COND_APPEND(driver_phone);
    SEARCH_COND_APPEND(plate_number);

    if (cond.status != 0)
    {
        query_cond += " AND status == " + std::to_string(cond.status);
    }
    else if (cond.exp_status != 0)
    {
        query_cond += " AND status != " + std::to_string(cond.exp_status);
    }

    auto es = sqlite_orm::search_record_all<sql_order>("%s ORDER BY datetime(p_time) DESC LIMIT 20 OFFSET %d", query_cond.c_str(), cond.page_no * 20);
    for (auto &itr : es)
    {
        vehicle_order_info tmp;
        db_2_rpc(itr, tmp);
        _return.push_back(tmp);
    }
}

void order_center_handler::get_order(vehicle_order_info &_return, const std::string &order_number)
{

    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }
    db_2_rpc(*es, _return);
}

void order_center_handler::get_registered_order(std::vector<vehicle_order_info> &_return)
{
    auto reg_infos = sqlite_orm::search_record_all<sql_order>("status != 100 AND reg_no != 0 ORDER BY datetime(reg_info_time)");
    for (auto &itr : reg_infos)
    {
        vehicle_order_info tmp;
        db_2_rpc(itr, tmp);
        _return.push_back(tmp);
    }
}

bool order_center_handler::order_check_in(const std::string &order_number, const bool is_check_in, const std::string &opt_name)
{
    bool ret = false;

    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }

    if (is_check_in)
    {
        es->reg_info_name = opt_name;
        es->reg_info_time = util_get_timestring();
        es->reg_no = gen_reg_no();
    }
    else if (es->status == 1)
    {
        es->reg_info_name = "";
        es->reg_info_time = "";
        es->reg_no = 0;
        es->call_info_name = "";
        es->call_info_time = "";
        es->confirm_info_name = "";
        es->confirm_info_time = "";
    }
    ret = es->update_record();
    auto_call_next();

    return ret;
}

bool order_center_handler::order_call(const std::string &order_number, const bool is_call, const std::string &opt_name)
{
    bool ret = false;
    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }
    if (es->reg_info_time.length() <= 0)
    {
        ZH_RETURN_MSG("车辆未排号");
    }
    if (is_call)
    {
        es->call_info_name = opt_name;
        es->call_info_time = util_get_timestring();
        zyzl_plugin::get_inst()->push_call(es->plate_number, es->driver_name);
    }
    else if (es->status == 1)
    {
        es->confirm_info_name = "";
        es->confirm_info_time = "";
        es->call_info_name = "";
        es->call_info_time = "";
        es->update_record();
        auto_call_next();
    }
    ret = es->update_record();
    return ret;
}

bool order_center_handler::order_confirm(const std::string &order_number, const bool is_confirm, const std::string &opt_name)
{
    bool ret = false;

    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }
    if (es->call_info_time.length() <= 0)
    {
        ZH_RETURN_MSG("车辆未叫号");
    }

    if (is_confirm)
    {
        es->confirm_info_name = opt_name;
        es->confirm_info_time = util_get_timestring();
    }
    else
    {
        es->confirm_info_name = "";
        es->confirm_info_time = "";
    }
    ret = es->update_record();

    return ret;
}

bool order_center_handler::order_set_seal_no(const std::string &order_number, const std::string &seal_no)
{
    bool ret = false;
    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }
    es->seal_no = seal_no;
    ret = es->update_record();

    return ret;
}

bool order_center_handler::order_push_weight(const std::string &order_number, const double weight, const std::string &opt_name)
{
    bool ret = false;
    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }
    auto cur_date = util_get_timestring();
    auto eph = es->get_children<sql_order_history>("belong_order", "node_name == '%s'", node_name_p_weight.c_str());
    auto emh = es->get_children<sql_order_history>("belong_order", "node_name == '%s'", node_name_m_weight.c_str());
    auto eih = es->get_children<sql_order_history>("belong_order", "node_name == '%s'", node_name_enter.c_str());
    auto eoh = es->get_children<sql_order_history>("belong_order", "node_name == '%s'", node_name_exit.c_str());
    if (eoh || es->status == 100)
    {
        ZH_RETURN_MSG("已出厂无法记录重量");
    }
    es->status = 2;
    if (es->confirm_info_time.length() <= 0)
    {
        if (eph)
        {
            eph->node_caller = opt_name;
            eph->occour_time = cur_date;
            eph->update_record();
        }
        else
        {
            sql_order_history tmp;
            tmp.node_caller = opt_name;
            tmp.occour_time = cur_date;
            tmp.node_name = node_name_p_weight;
            tmp.set_parent(*es, "belong_order");
            tmp.insert_record();
        }
        es->p_weight = weight;
        es->p_time = cur_date;
    }
    else
    {
        if (!eph)
        {
            sql_order_history tmp;
            tmp.node_caller = opt_name;
            tmp.occour_time = cur_date;
            tmp.node_name = node_name_p_weight;
            tmp.set_parent(*es, "belong_order");
            tmp.insert_record();
            es->p_weight = weight;
            es->p_time = cur_date;
        }
        else
        {
            if (emh)
            {
                emh->node_caller = opt_name;
                emh->occour_time = cur_date;
                emh->update_record();
            }
            else
            {
                sql_order_history tmp;
                tmp.node_caller = opt_name;
                tmp.occour_time = cur_date;
                tmp.node_name = node_name_m_weight;
                tmp.set_parent(*es, "belong_order");
                tmp.insert_record();
            }
            es->m_weight = weight;
            es->m_time = cur_date;
            es->update_record();
            if (!eih)
            {
                close_order(*es);
            }
        }
    }

    ret = es->update_record();

    return ret;
}

bool order_center_handler::order_rollback_weight(const std::string &order_number, const std::string &opt_name)
{
    return false;
}

bool order_center_handler::order_push_gate(const std::string &order_number, const std::string &opt_name)
{
    bool ret = false;
    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }
    auto eih = es->get_children<sql_order_history>("belong_order", "node_name == '%s'", node_name_enter.c_str());
    auto eoh = es->get_children<sql_order_history>("belong_order", "node_name == '%s'", node_name_exit.c_str());
    auto emh = es->get_children<sql_order_history>("belong_order", "node_name == '%s'", node_name_m_weight.c_str());
    auto eph = es->get_children<sql_order_history>("belong_order", "node_name == '%s'", node_name_p_weight.c_str());
    if (eoh || es->status == 100)
    {
        ZH_RETURN_MSG("已出厂无法再次出入");
    }

    auto cur_date = util_get_timestring();
    es->status = 2;

    if (eih)
    {
        if (eph && !emh)
        {
            ZH_RETURN_MSG("车辆未二次称重");
        }
        sql_order_history tmp;
        tmp.node_caller = opt_name;
        tmp.node_name = node_name_exit;
        tmp.occour_time = cur_date;
        tmp.set_parent(*es, "belong_order");
        tmp.insert_record();
        close_order(*es);
    }
    else
    {
        if (!eph && !emh)
        {
            sql_order_history tmp;
            tmp.node_caller = opt_name;
            tmp.node_name = node_name_enter;
            tmp.occour_time = cur_date;
            tmp.set_parent(*es, "belong_order");
            tmp.insert_record();
        }
    }
    ret = es->update_record();

    return ret;
}

bool order_center_handler::order_rollback_gate(const std::string &order_number, const std::string &opt_name)
{
    return false;
}

bool order_center_handler::order_push_attach(const std::string &order_number, const std::string &name, const std::string &att_path)
{
    bool ret = false;

    auto es = get_order_by_number(order_number);
    if (!es)
    {
        ZH_RETURN_NO_ORDER();
    }
    sql_order_attach tmp;
    tmp.att_name = name;
    tmp.att_path = att_path;
    tmp.set_parent(*es, "belong_order");

    ret = tmp.insert_record();

    return ret;
}