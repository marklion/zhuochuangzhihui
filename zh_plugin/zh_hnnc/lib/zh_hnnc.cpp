#include "zh_hnnc.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include "httplib.h"
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#define PLUGIN_CONF_FILE "/plugin/zh_hnnc/conf/plugin.json"

static tdf_log g_log("hnnc_audit", "/plugin/audit.log", "/plugin/audit.log");

static void get_token()
{
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    httplib::Client cli(config("remote_url"));
    httplib::Params hps = {{"username", config("username")},
                           {"password", config("password")}};
    auto res = cli.Post("/rest/tokens", hps);
    if (res)
    {
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "token", res->body);
    }
    else
    {
        g_log.err("failed to post token req");
    }
}

static bool send_req_with_token(const neb::CJsonObject _req, const std::function<bool(const neb::CJsonObject &)> &_callback, bool _once_again = false)
{
    bool ret = false;

    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    if (config.KeyExist("token"))
    {
        httplib::Client cli(config("remote_url"));
        cli.set_read_timeout(8, 0);
        httplib::Headers ref = {{"X-AUTH-TOKEN", config("token")}};
        cli.set_default_headers(ref);
        auto begin_point = time(NULL);
        g_log.log("send req to hn:%s", _req.ToFormattedString().c_str());
        auto res = cli.Post("/rest/apiController.do?hnHgtsItf2", _req.ToString(), "application/json");
        if (res)
        {
            auto res_json = neb::CJsonObject(res->body);
            g_log.log("recv resp from hn:%s", res_json.ToFormattedString().c_str());
            if (res_json("success") == "true")
            {
                ret = _callback(res_json["obj"]);
            }
            else
            {
                g_log.err("failure because %s", res_json("msg").c_str());
                std::cerr << res_json("msg") << std::endl;
            }
        }
        else
        {
            g_log.err("failed to post api req");
        }
        auto end_point = time(NULL);
        g_log.log("spend %d second", end_point - begin_point);
    }
    if (false == ret && _once_again == false)
    {
        if (_once_again == false)
        {
            g_log.log("post try again");
            get_token();
            ret = send_req_with_token(_req, _callback, true);
        }
    }

    return ret;
}

static bool post_req_and_print(const neb::CJsonObject &req)
{
    return send_req_with_token(
        req,
        [](const neb::CJsonObject &res) -> bool
        {
        std::cout << res.ToFormattedString() << std::endl;
        return true; });
}

bool ZH_HNNC_fetch_cor_info()
{
    bool ret = false;

    neb::CJsonObject req;
    req.Add("transtype", "GetCorp");
    ret = post_req_and_print(req);

    return ret;
}
bool ZH_HNNC_fetch_customer_info()
{
    bool ret = false;

    neb::CJsonObject req;
    req.Add("transtype", "GetCust");
    req.Add("Corpcode", zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("corpcode"));
    ret = post_req_and_print(req);

    return ret;
}
bool ZH_HNNC_fetch_order_info(const std::string &_cust_code)
{
    bool ret = false;

    neb::CJsonObject req;
    req.Add("transtype", "GetOrder");
    req.Add("Corpcode", zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("corpcode"));
    req.Add("Custcode", _cust_code);

    ret = post_req_and_print(req);

    return ret;
}

bool ZH_HNNC_valid_balance(const std::string &cust_code, const std::string &inv_code, int vehicle_count)
{
    bool ret = false;

    neb::CJsonObject req;
    req.Add("transtype", "CheckBalance");
    req.Add("Corpcode", zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("corpcode"));
    req.Add("Custcode", cust_code);
    req.Add("Invcode", inv_code);
    req.Add("Num", vehicle_count);

    ret = send_req_with_token(req, [](const neb::CJsonObject &res) -> bool
                              { return true; });

    return ret;
}

bool ZH_HNNC_push_req(const std::string &_req_json_string)
{
    bool ret = false;

    neb::CJsonObject req(_req_json_string);
    req.Add("Corpcode", zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("corpcode"));

    ret = post_req_and_print(req);

    return ret;
}