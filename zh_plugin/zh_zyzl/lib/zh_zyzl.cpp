#include "zh_zyzl.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#define PLUGIN_CONF_FILE "/plugin/zh_zyzl/conf/plugin.json"

static tdf_log g_log("zyzl", "/plugin/audit.log", "/plugin/audit.log");

static bool send_to_zyzl(const std::string &_path, const neb::CJsonObject &_req, const std::function<bool(const neb::CJsonObject &)> &_callback)
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

        auto res = cli.Post(real_path.c_str(),  real_req.ToString(), "application/json");
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
        [&](const neb::CJsonObject &res) -> bool {
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
        [&](const neb::CJsonObject &res) -> bool {
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

    if (send_to_zyzl(push_p_path, req, [&](const neb::CJsonObject &res)->bool{
        return true;
    }))
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
bool ZH_ZYZL_push_weight(const std::string &_plate, const std::string &_p_time, const std::string &_m_time, double p_weight, double m_weight, double j_weight)
{
    bool ret = false;
    std::string push_weight_path = "/push_weight";

    neb::CJsonObject req;
    req.Add("id", get_id_from_plate(_plate));
    req.Add("plateNo", _plate);
    req.Add("pWeight", p_weight);
    req.Add("mWeight", m_weight);
    req.Add("jWeight", j_weight);
    req.Add("pTime", _p_time);
    req.Add("mTime", _m_time);

    if (send_to_zyzl(push_weight_path, req, [&](const neb::CJsonObject &res) -> bool
                     { return true; }))
    {
        ret = true;
    }

    return ret;
}