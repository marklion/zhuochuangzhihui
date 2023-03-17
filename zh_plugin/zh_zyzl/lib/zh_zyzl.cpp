#include "zh_zyzl.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#define PLUGIN_CONF_FILE "/plugin/zh_zyzl/conf/plugin.json"

static tdf_log g_log("zyzl", "/plugin/audit.log", "/plugin/audit.log");

static bool send_to_zyzl(const std::string &_path, const neb::CJsonObject &_req, const std::function<bool(const neb::CJsonObject &)> &_callback, bool _is_get = false)
{
    bool ret = false;
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto token = config("token");
    auto remote_url = config("zyzl_url");
    if (token.length() > 0 && remote_url.length() > 0)
    {
        httplib::Client cli(remote_url);
        cli.set_read_timeout(20, 0);
        cli.set_follow_location(true);
        auto begin_point = time(NULL);
        auto real_req = _req;

        std::string real_path = "/pa_web/pa_rest" + _path + "?token=" + token;
        g_log.log("send req to host->%s path->%s token->%s:%s", remote_url.c_str(), real_path.c_str(), token.c_str(), real_req.ToFormattedString().c_str());

        if (_is_get)
        {
            auto res = cli.Get(real_path.c_str());
            if (res)
            {
                auto res_json = neb::CJsonObject(res->body);
                g_log.log("recv resp from zyzl:%s", res_json.ToFormattedString().c_str());
                if (res_json("err_msg") == "")
                {
                    ret = _callback(res_json["result"]);
                }
                else
                {
                    g_log.err("failure because %s", res_json("err_msg").c_str());
                    std::cerr << res_json("err_msg") << std::endl;
                }
            }
            else
            {
                g_log.err("failed to post api req :%s", httplib::to_string(res.error()).c_str());
            }
        }
        else
        {
            auto res = cli.Post(real_path.c_str(), real_req.ToString(), "application/json");
            if (res)
            {
                auto res_json = neb::CJsonObject(res->body);
                g_log.log("recv resp from zyzl:%s", res_json.ToFormattedString().c_str());
                if (res_json("err_msg") == "")
                {
                    ret = _callback(res_json["result"]);
                }
                else
                {
                    g_log.err("failure because %s", res_json("err_msg").c_str());
                    std::cerr << res_json("err_msg") << std::endl;
                }
            }
            else
            {
                g_log.err("failed to post api req :%s", httplib::to_string(res.error()).c_str());
            }
        }

        auto end_point = time(NULL);
        g_log.log("spend %d second", end_point - begin_point);
    }
    else
    {
        g_log.err("no access code or url");
    }

    return ret;
}

static std::string get_id_from_plate(const std::string &_plate)
{
    std::string ret;

    neb::CJsonObject req;
    req.Add("plateNo", _plate);

    send_to_zyzl(
        "/vehicle_info",
        req,
        [&](const neb::CJsonObject &res) -> bool
        {
            bool lamda_ret = false;

            if (res.KeyExist("id"))
            {
                ret = res("id");
                lamda_ret = true;
            }

            return lamda_ret;
        });

    return ret;
}
static std::string get_driver_name_from_plate(const std::string &_plate)
{
    std::string ret;

    neb::CJsonObject req;
    req.Add("plateNo", _plate);

    send_to_zyzl(
        "/vehicle_info",
        req,
        [&](const neb::CJsonObject &res) -> bool
        {
            bool lamda_ret = false;

            if (res.KeyExist("driverName"))
            {
                ret = res("driverName");
                lamda_ret = true;
            }

            return lamda_ret;
        });

    return ret;
}

bool ZH_ZYZL_push_p(const std::string &_plate)
{
    bool ret = false;
    std::string push_p_path = "/push_p";

    neb::CJsonObject req;
    req.Add("id", get_id_from_plate(_plate));

    if (send_to_zyzl(push_p_path, req, [&](const neb::CJsonObject &res) -> bool
                     { return true; }))
    {
        ret = true;
    }

    return ret;
}
bool ZH_ZYZL_call_vehicle(const std::string &_plate)
{
    bool ret = false;
    std::string call_vehicle_path = "/call_vehicle";

    neb::CJsonObject req;
    req.Add("plateNo", _plate);
    req.Add("driverName", get_driver_name_from_plate(_plate));

    if (send_to_zyzl(call_vehicle_path, req, [&](const neb::CJsonObject &res) -> bool
                     { return true; }))
    {
        ret = true;
    }

    return ret;
}
static std::string _util_get_timestring(time_t _time)
{
    time_t cur_time = _time;

    auto st_time = localtime(&cur_time);
    char buff[512] = "";

    sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", st_time->tm_year + 1900, st_time->tm_mon + 1, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);

    return std::string(buff);
}
static std::string _util_get_datestring(time_t _time)
{
    auto date_time = _util_get_timestring(_time);
    return date_time.substr(0, 10);
}
bool ZH_ZYZL_push_weight(const std::string &_plate, const std::string &_p_time, const std::string &_m_time, double p_weight, double m_weight, double j_weight, const std::string &_ticket_no, const std::string &_seal_no)
{
    bool ret = false;
    std::string push_weight_path = "/push_weight";

    neb::CJsonObject req;
    auto zyzl_id = get_id_from_plate(_plate);
    req.Add("id", zyzl_id);
    req.Add("plateNo", _plate);
    req.Add("pWeight", p_weight);
    req.Add("mWeight", m_weight);
    req.Add("jWeight", j_weight);
    req.Add("pTime", _p_time);
    req.Add("mTime", _m_time);
    req.Add("ticketNo", _ticket_no);
    req.Add("sealNo", _seal_no);

    if (send_to_zyzl(
            push_weight_path,
            req,
            [&](const neb::CJsonObject &res) -> bool
            { return true; }))
    {

        std::string file_name = "/tmp/ticket_" + _plate + "_" + _util_get_datestring(time(nullptr)) + ".png";

        if (zyzl_id.length() > 0)
        {
            std::string pull_cmd = "[ -f /root/store_file.sh ] && /root/store_file.sh " + zyzl_id+ " " + file_name + " || echo 123";
            if (0 == system(pull_cmd.c_str()))
            {
                ret = true;
            }
        }
    }

    return ret;
}

static bool create_plan_to_remote(const zy_sync_plan_data &_plan)
{
    bool ret = false;
    neb::CJsonObject req;
    req.Add("plateNo", _plan.planNo);
    req.Add("behindPlateNo", _plan.behindPlateNo);
    req.Add("driverName", _plan.driverName);
    req.Add("driverPhone", _plan.driverPhone);
    req.Add("driverID", _plan.driverID);
    req.Add("useFor", _plan.useFor);
    req.Add("deliverAddress", _plan.deliverAddress);
    req.Add("arriveDate", _plan.arriveDate);
    req.Add("customerName", _plan.customerName);
    req.Add("stuffName", _plan.stuffName);
    req.Add("trans_company_name", _plan.trans_company_name);
    send_to_zyzl("/create_plan", req, [&](const neb::CJsonObject &_resp) -> bool
                 {
        ret = true;
        return true; });

    return ret;
}

static void cancel_plan_from_remote(const std::string &_order_number)
{
    neb::CJsonObject req;
    req.Add("orderNumber", _order_number);
    send_to_zyzl("/cancel_plan", req, [](const neb::CJsonObject &_resp) -> bool
                 { return true; });
}

zy_sync_plan_data json_to_struct_plan(const neb::CJsonObject &_json)
{
    zy_sync_plan_data tmp;
    tmp.arriveDate = _json("createTime").substr(0, 10);
    tmp.behindPlateNo = _json("backPlateNo");
    tmp.customerName = _json("companyName");
    tmp.deliverAddress = _json("sale_address");
    tmp.driverID = _json("driverId");
    tmp.driverName = _json("driverName");
    tmp.driverPhone = _json("driverPhone");
    tmp.planNo = _json("plateNo");
    tmp.stuffName = _json("stuffName");
    tmp.useFor = "气站";
    tmp.order_number = _json("orderNo");
    tmp.trans_company_name = _json("transCompanyName");

    return tmp;
}

void ZH_ZYZL_sync_plans(const std::list<zy_sync_plan_data> &_plan_data)
{
    std::list<zy_sync_plan_data> orig_remote_plans;
    if (false == send_to_zyzl(
                     "/all_vehicle_info",
                     neb::CJsonObject(),
                     [&](const neb::CJsonObject &_resp) -> bool
                     {
                         auto remote_json = _resp;
                         for (auto i = 0; i < remote_json.GetArraySize(); i++)
                         {
                             auto single_plan = remote_json[i];
                             orig_remote_plans.push_back(json_to_struct_plan(single_plan));
                         }

                         return true;
                     },
                     true))
    {
        return;
    }

    for (auto &itr : _plan_data)
    {
        if (std::find(orig_remote_plans.begin(), orig_remote_plans.end(), itr) == orig_remote_plans.end())
        {
            create_plan_to_remote(itr);
        }
    }

    for (auto &itr : orig_remote_plans)
    {
        if (std::find_if(_plan_data.begin(), _plan_data.end(), [&](const zy_sync_plan_data &_item)
                         {
            bool found_ret = false;
            if (_item == itr && _item.arriveDate == itr.arriveDate)
            {
                found_ret = true;
            }

            return found_ret; }) == _plan_data.end())
        {
            auto order_id = itr.order_number.substr(std::to_string(time(nullptr)).length());
            cancel_plan_from_remote(order_id);
        }
    }
}

std::string ZH_ZYZL_pull_ticket(const std::string &_plate)
{
    std::string ret;
    std::string file_name = "/tmp/ticket_" + _plate + "_" + _util_get_datestring(time(nullptr)) + ".png";

    if (0 == access(file_name.c_str(), R_OK))
    {
        ret = file_name;
    }

    return ret;
}