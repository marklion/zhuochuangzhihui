#include "zh_meiyitong.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include "httplib.h"
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#define PLUGIN_CONF_FILE "/plugin/zh_meiyitong/conf/plugin.json"

static tdf_log g_log("meiyitong_audit", "/plugin/audit.log", "/plugin/audit.log");

static bool send_to_myt(const std::string &_path, const neb::CJsonObject &_req, const std::function<bool(const neb::CJsonObject &)> &_callback)
{
    bool ret = false;
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto access_code = config("access_code");
    auto remote_url = config("remote_url");
    if (access_code.length() > 0 && remote_url.length() > 0)
    {
        httplib::Client cli(remote_url);
        cli.set_read_timeout(8, 0);
        auto begin_point = time(NULL);
        auto real_req = _req;
        real_req.Add("AccessCode",access_code);
        g_log.log("send req to host->%s path->%s:%s",remote_url.c_str(), _path.c_str(), real_req.ToFormattedString().c_str());
        auto res = cli.Post(_path.c_str(), real_req.ToString(), "application/json");
        if (res)
        {
            auto res_json = neb::CJsonObject(res->body);
            g_log.log("recv resp from meiyitong:%s", res_json.ToFormattedString().c_str());
            if (res_json("ok") == "true")
            {
                ret = _callback(res_json["obj"]);
            }
            else
            {
                g_log.err("failure because %s", res_json["result"].ToString().c_str());
            }
        }
        else
        {
            g_log.err("failed to post api req");
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

static bool send_req_and_print_res(const std::string &_path, const neb::CJsonObject &_req)
{
    return send_to_myt(_path, _req, [](const neb::CJsonObject &res)->bool{
        std::cout << res.ToFormattedString() << std::endl;
        return true;
    });
}

bool ZH_MEIYITONG_get_dest_addr()
{
    bool ret = false;

    ret = send_req_and_print_res("/supervisory_division_restful", neb::CJsonObject());

    return ret;
}