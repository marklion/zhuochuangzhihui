#include "zh_zyhl.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"

#define PLUGIN_CONF_FILE "/plugin/zh_zyhl/conf/plugin.json"
static tdf_log g_log("zyhl", "/plugin/audit.log", "/plugin/audit.log");

static bool send_req_to_zyhl(const std::string &_url, const neb::CJsonObject &_req, const std::function<bool(const neb::CJsonObject &)> &_callback)
{
    bool ret = false;
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto appid = config("appid");
    auto secret = config("secret");
    auto remote_url = config("zyhl_url");
    if (appid.length() > 0 && secret.length() > 0 && remote_url.length() > 0)
    {
        httplib::Client cli(config("zyhl_url_host"));
        cli.set_read_timeout(20, 0);
        cli.set_follow_location(true);
        auto begin_point = time(NULL);

        std::string token_req_path = remote_url + "/thirdparty/access_token";
        neb::CJsonObject token_req;
        token_req.Add("appid", appid);
        token_req.Add("secret", secret);
        g_log.log("send req to host->%s, body:%s", token_req_path.c_str(), token_req.ToFormattedString().c_str());
        auto token_res = cli.Post(token_req_path.c_str(), token_req.ToString(), "application/json");
        if (token_res)
        {
            auto res_json = neb::CJsonObject(token_res->body);
            g_log.log("recv resp from zyhl:%s", res_json.ToFormattedString().c_str());
            if (res_json("code") == "0")
            {
                httplib::Headers ref = {{"token", res_json["data"]("token")}};
                cli.set_default_headers(ref);
                std::string real_path = remote_url + _url;
                auto real_req = _req;
                g_log.log("send req to host->%s, body:%s", real_path.c_str(), real_req.ToFormattedString().c_str());

                auto res = cli.Post(real_path.c_str(), real_req.ToString(), "application/json");
                if (res)
                {
                    auto res_json = neb::CJsonObject(res->body);
                    g_log.log("recv resp from zyhl:%s", res_json.ToFormattedString().c_str());
                    if (res_json("code") == "0")
                    {
                        ret = _callback(res_json["data"]);
                    }
                    else
                    {
                        g_log.err("failure because %s", res_json("message").c_str());
                        std::cerr << res_json("message") << std::endl;
                    }
                }
                else
                {
                    g_log.err("failed to post api req :%s", httplib::to_string(res.error()).c_str());
                }
            }
            else
            {
                g_log.err("failed to get token because:%s", res_json("message").c_str());
            }
        }
        else
        {
            g_log.err("failed to post api req :%s", httplib::to_string(token_res.error()).c_str());
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

void fetch_plan_from_zyhl(const std::string &_date)
{
    neb::CJsonObject get_plan_req;
    get_plan_req.Add("date", _date);
    auto date_ystd = util_get_datestring(time(nullptr) - (3600*24));
    neb::CJsonObject fetch_req;
    std::list<neb::CJsonObject> req_list;
    auto collect_plan = [&](const neb::CJsonObject _response) -> bool
    {
        auto resp = _response;
        for (auto i = 0; i < resp.GetArraySize(); i++)
        {
            auto single_item = resp[i];
            if (single_item.IsNull("number"))
            {
                neb::CJsonObject tmp;
                tmp.Add("plateNo", single_item("driver_no"));
                tmp.Add("backPlateNo", single_item("driver_no2"));
                tmp.Add("driverName", single_item("driver_name"));
                tmp.Add("driverPhone", single_item("driver_tel"));
                tmp.Add("driverId", single_item(""));
                tmp.Add("useFor", "气站");
                tmp.Add("sale_address", single_item["product"]("area"));
                tmp.Add("createTime", single_item("delivery_date"));
                tmp.Add("companyName", single_item["customer_company"]("name"));
                tmp.Add("stuffName", single_item["product"]("type"));
                req_list.push_back(tmp);
            }
        }

        return true;
    };
    auto send_ret = send_req_to_zyhl("/thirdparty/list_plan", get_plan_req, collect_plan);
    if (zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("need_ystd") == "true")
    {
        get_plan_req.ReplaceAdd("date", date_ystd);
        send_ret = send_req_to_zyhl("/thirdparty/list_plan", get_plan_req, collect_plan);
    }
    if (send_ret)
    {
        req_list.sort(
            [](const neb::CJsonObject &_fir, const neb::CJsonObject &_sec)
            {
                return _fir("plateNo") < _sec("plateNo");
            });
        req_list.unique(
            [](const neb::CJsonObject &_fir, const neb::CJsonObject &_sec)
            {
                return _fir("plateNo") == _sec("plateNo");
            });
        for (auto &single_req : req_list)
        {
            fetch_req.Add(single_req);
        }
        g_log.log("call %s with args %s", "/plugin/zh_zyzl/bin/zh_zyzl_plugin sync_plan", fetch_req.ToString().c_str());
        auto sub_proc_stdin = popen("/plugin/zh_zyzl/bin/zh_zyzl_plugin sync_plan", "w");
        if (sub_proc_stdin)
        {
            if (0 >= fwrite(fetch_req.ToString().c_str(), fetch_req.ToString().size(), 1, sub_proc_stdin))
            {
                g_log.err("failed to write req to sub process:%s", strerror(errno));
            }
            pclose(sub_proc_stdin);
        }
    }
}
static std::string util_get_timestring(time_t _time)
{
    time_t cur_time = _time;

    auto st_time = localtime(&cur_time);
    char buff[512] = "";

    sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", st_time->tm_year + 1900, st_time->tm_mon + 1, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);

    return std::string(buff);
}
std::string util_get_datestring(time_t _time)
{
    auto date_time = util_get_timestring(_time);
    return date_time.substr(0, 10);
}

static neb::CJsonObject get_zyhl_plans()
{
    neb::CJsonObject cur_plans;
    neb::CJsonObject get_plan_req;
    auto date_ystd = util_get_datestring(time(nullptr) - (3600 * 24));
    get_plan_req.Add("date", date_ystd);
    auto plan_proc_resp = [&](const neb::CJsonObject &_resp) -> bool
    {
        auto resp = _resp;
        for (auto i = 0; i < resp.GetArraySize(); i++)
        {
            cur_plans.Add(resp[i]);
        }
        return true;
    };
    if (zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("need_ystd") == "true")
    {
        send_req_to_zyhl(
            "/thirdparty/list_plan",
            get_plan_req,
            plan_proc_resp);
    }
    get_plan_req.Replace("date", util_get_datestring(time(nullptr)));
    send_req_to_zyhl(
        "/thirdparty/list_plan",
        get_plan_req,
        plan_proc_resp);
    return cur_plans;
}

bool push_vehicle_enter(const std::string &_vehicle_number, double _xxx)
{
    bool ret = false;
    auto cur_plans = get_zyhl_plans();

    for (auto i = 0; i < cur_plans.GetArraySize(); i++)
    {
        auto &single_plan = cur_plans[i];
        if (single_plan("driver_no") == _vehicle_number && single_plan.IsNull("number"))
        {
            neb::CJsonObject enter_req;
            enter_req.Add("plan_id", single_plan("id"));
            if (send_req_to_zyhl("/thirdparty/enter", enter_req, [](const neb::CJsonObject &_enter_resp) -> bool
                                 { return true; }))
            {
                ret = true;
            }
            break;
        }
    }
    ret = true;

    return ret;
}

std::string send_file_to_zyhl(const std::string &_id, const std::string &_path)
{
    std::string ret;
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto appid = config("appid");
    auto secret = config("secret");
    auto remote_url = config("zyhl_url");
    if (appid.length() > 0 && secret.length() > 0 && remote_url.length() > 0)
    {
        httplib::Client cli(config("zyhl_url_host"));
        cli.set_read_timeout(20, 0);
        cli.set_follow_location(true);
        auto begin_point = time(NULL);

        std::string token_req_path = remote_url + "/thirdparty/access_token";
        neb::CJsonObject token_req;
        token_req.Add("appid", appid);
        token_req.Add("secret", secret);
        g_log.log("send req to host->%s, body:%s", token_req_path.c_str(), token_req.ToFormattedString().c_str());
        auto token_res = cli.Post(token_req_path.c_str(), token_req.ToString(), "application/json");
        if (token_res)
        {
            auto res_json = neb::CJsonObject(token_res->body);
            g_log.log("recv resp from zyhl:%s", res_json.ToFormattedString().c_str());
            if (res_json("code") == "0")
            {
                httplib::Headers ref = {{"token", res_json["data"]("token")}};
                cli.set_default_headers(ref);
                std::string real_path = remote_url + "/file/upload?img=1";
                g_log.log("send file to host->%s, file:%s", real_path.c_str(), _path.c_str());
                std::ifstream tmp_stream(_path);
                std::stringstream buff_tmp_stream;
                buff_tmp_stream << tmp_stream.rdbuf();
                httplib::MultipartFormDataItems items = {
                    {"files", buff_tmp_stream.str(), "ticket.png", "application/octet-stream"}};
                auto res = cli.Post(real_path.c_str(), items);
                if (res)
                {
                    auto res_json = neb::CJsonObject(res->body);
                    g_log.log("recv resp from zyhl:%s", res_json.ToFormattedString().c_str());
                    if (res_json("code") == "0")
                    {
                        ret = res_json["data"](0);
                    }
                    else
                    {
                        g_log.err("failure because %s", res_json("message").c_str());
                        std::cerr << res_json("message") << std::endl;
                    }
                }
                else
                {
                    g_log.err("failed to post api req :%s", httplib::to_string(res.error()).c_str());
                }
            }
            else
            {
                g_log.err("failed to get token because:%s", res_json("message").c_str());
            }
        }
        else
        {
            g_log.err("failed to post api req :%s", httplib::to_string(token_res.error()).c_str());
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
static std::string get_attch_name(const std::string &_vehicle_number)
{
    std::string ret;
    std::string pull_cmd = "/plugin/zh_zyzl/bin/zh_zyzl_plugin pull_ticket " + _vehicle_number;
    g_log.log("call %s", pull_cmd.c_str());
    auto path_file = popen(pull_cmd.c_str(), "r");
    if (path_file)
    {
        char file_name_buff[4096] = "";
        fread(file_name_buff, sizeof(file_name_buff), 1, path_file);
        pclose(path_file);
        file_name_buff[strlen(file_name_buff) - 1] = 0;
        g_log.log("attach_filename:%s", file_name_buff);
        struct stat file_st;
        if (0 == stat(file_name_buff, &file_st))
        {
            if (file_st.st_size > 0)
            {
                auto first_size = file_st.st_size;
                sleep(20);
                if (0 == stat(file_name_buff, &file_st))
                {
                    if (file_st.st_size == first_size)
                    {
                        ret = file_name_buff;
                    }
                }
            }
        }
    }
    return ret;
}
bool push_vehicle_weight(const std::string &_vehicle_number, double _weight)
{
    bool ret = false;

    auto file_name_att = get_attch_name(_vehicle_number);
    if (file_name_att.length() <= 0)
    {
        return ret;
    }
    auto cur_plans = get_zyhl_plans();
    for (auto i = 0; i < cur_plans.GetArraySize(); i++)
    {
        auto &single_plan = cur_plans[i];
        if (single_plan("driver_no") == _vehicle_number && single_plan.IsNull("number"))
        {
            neb::CJsonObject load_req;
            load_req.Add("id", single_plan("id"));
            load_req.Add("load_number", _weight);
            load_req.Add("load_date", util_get_datestring(time(nullptr)));

            if (send_req_to_zyhl("/thirdparty/update_loadnumber", load_req, [](const neb::CJsonObject &_enter_resp) -> bool
                                 { return true; }))
            {
                auto file_id = send_file_to_zyhl(single_plan("id"), file_name_att);
                neb::CJsonObject bind_file_req;
                bind_file_req.Add("file", "https://lng.hy3416.com/51jiaye/api/file/download?id=" + file_id);
                bind_file_req.Add("id", single_plan("id"));
                if (send_req_to_zyhl("/thirdparty/add_invoice", bind_file_req, [](const neb::CJsonObject &) -> bool
                                     { return true; }))
                {
                    ret = true;
                }
            }
            break;
        }
    }

    return ret;
}