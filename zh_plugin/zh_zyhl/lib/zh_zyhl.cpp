#include "zh_zyhl.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#include <thread>

#define PLUGIN_CONF_FILE "/plugin/zh_zyhl/conf/plugin.json"
static tdf_log g_log("zyhl", "/plugin/audit.log", "/plugin/audit.log");

static std::vector<vehicle_order_detail> get_xy_vehicles()
{
    std::vector<vehicle_order_detail> ret;
    using namespace ::apache::thrift;
    using namespace ::apache::thrift::protocol;
    using namespace ::apache::thrift::transport;
    THR_DEF_CIENT(vehicle_order_center);
    THR_CONNECT(vehicle_order_center);
    client->get_today_xy_vehicle(ret);
    TRH_CLOSE();

    return ret;
}



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
std::string zh_double2string_reserve2(double _value)
{
    std::stringstream ss;
    ss.setf(std::ios::fixed);
    ss.precision(2);
    ss << _value;
    return ss.str();
}

static bool hl_vehicle_is_xy(const neb::CJsonObject &_hl_v, const std::vector<vehicle_order_detail> &_xy_vos)
{
    bool ret = false;
    struct vehicle_last_weight {
        std::string vehicle_number;
        double weight = 0;
    };
    double hl_weight = 0;
    _hl_v.Get("number", hl_weight);
    std::map<std::string, vehicle_last_weight> tmp_map;
    for (auto &itr : _xy_vos)
    {
        tmp_map[itr.basic_info.main_vehicle_number].weight += itr.basic_info.m_weight - itr.basic_info.p_weight;
    }
    for (auto itr = tmp_map.begin(); itr != tmp_map.end(); ++itr)
    {
        if (itr->first == _hl_v("driver_no") && zh_double2string_reserve2(itr->second.weight) == zh_double2string_reserve2(hl_weight))
        {
            ret = true;
            break;
        }
    }

    return ret;
}

void fetch_plan_from_zyhl(const std::string &_date)
{
    neb::CJsonObject get_plan_req;
    get_plan_req.Add("date", _date);
    auto date_ystd = util_get_datestring(time(nullptr) - (3600 * 24));
    neb::CJsonObject fetch_req;
    std::list<neb::CJsonObject> req_list;
    auto xy_vos = get_xy_vehicles();
    auto collect_plan = [&](const neb::CJsonObject _response) -> bool
    {
        auto resp = _response;
        for (auto i = 0; i < resp.GetArraySize(); i++)
        {
            auto single_item = resp[i];
            bool should_be_sync = false;
            if (single_item.IsNull("number"))
            {
                should_be_sync = true;
            }
            else if (hl_vehicle_is_xy(single_item, xy_vos))
            {
                should_be_sync = true;
            }
            if (should_be_sync)
            {
                neb::CJsonObject tmp;
                std::string company_name =single_item["customer_company"]("name") ;
                tmp.Add("plateNo", single_item("driver_no"));
                tmp.Add("backPlateNo", single_item("driver_no2"));
                tmp.Add("driverName", single_item("driver_name"));
                tmp.Add("driverPhone", single_item("driver_tel"));
                tmp.Add("driverId", single_item(""));
                tmp.Add("useFor", "气站");
                tmp.Add("sale_address", single_item["product"]("area"));
                tmp.Add("createTime", single_item("delivery_date"));
                tmp.Add("companyName", company_name.substr(company_name.find_first_not_of("贸易-")));
                tmp.Add("stuffName", single_item["product"]("type"));
                tmp.Add("transCompanyName", single_item["transport_company"]("name"));
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
    auto xy_vos = get_xy_vehicles();
    for (auto i = 0; i < cur_plans.GetArraySize(); i++)
    {
        auto &single_plan = cur_plans[i];
        if (single_plan("driver_no") == _vehicle_number)
        {
            if (hl_vehicle_is_xy(single_plan, xy_vos))
            {
                ret = true;
                break;
            }
            else if (single_plan.IsNull("number"))
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
    }

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
    auto xy_vos = get_xy_vehicles();

    auto file_name_att = get_attch_name(_vehicle_number);
    if (file_name_att.length() <= 0)
    {
        return ret;
    }
    auto cur_plans = get_zyhl_plans();
    for (auto i = 0; i < cur_plans.GetArraySize(); i++)
    {
        auto &single_plan = cur_plans[i];
        if (single_plan("driver_no") == _vehicle_number)
        {
            neb::CJsonObject load_req;
            load_req.Add("id", single_plan("id"));
            load_req.Add("load_number", _weight);
            load_req.Add("load_date", util_get_datestring(time(nullptr)));

            if (hl_vehicle_is_xy(single_plan, xy_vos))
            {
                double last_weight = 0;
                single_plan.Get("number", last_weight);
                load_req.ReplaceAdd("load_number", _weight + last_weight);
            }
            else if (!single_plan.IsNull("number") || single_plan.IsNull("carEntery"))
            {
                continue;
            }
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
time_t util_get_time_by_string(const std::string &_time_string)
{
    const char *cha = _time_string.data();                                    // 将string转换成char*。
    tm tm_ = {0};                                                             // 定义tm结构体。
    int year, month, day;                                     // 定义时间的各个int临时变量。
    sscanf(cha, "%d-%d-%d", &year, &month, &day); // 将string存储的日期时间，转换为int临时变量。
    tm_.tm_year = year - 1900;                                                // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
    tm_.tm_mon = month - 1;                                                   // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
    tm_.tm_mday = day;                                                        // 日。
    tm_.tm_isdst = 0;                                                         // 非夏令时。
    time_t t_ = mktime(&tm_);                                                 // 将tm结构体转换成time_t格式。
    return t_;                                                                // 返回值。
}
void get_zip_ticket(const std::string &_begin_date, const std::string &_end_date, const std::string &_trans_comapny_name)
{
    zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "download_status", "正在下载");
    auto begin_date_time = util_get_time_by_string(_begin_date);
    auto end_date_time = util_get_time_by_string(_end_date);
    struct ticket_meta{
        std::string date;
        std::string vehicle_number;
        std::string trans_company;
        std::vector<std::string> ticket_path;
    };
    std::list<ticket_meta> all_ticket;
    for (auto tmp_time = begin_date_time; tmp_time <= end_date_time; tmp_time += 3600 * 24)
    {
        neb::CJsonObject get_plan_req;
        get_plan_req.ReplaceAdd("date", util_get_datestring(tmp_time));
        send_req_to_zyhl(
            "/thirdparty/list_plan",
            get_plan_req,
            [&](const neb::CJsonObject &_resp) -> bool
            {
                auto resp = _resp;
                for (auto i = 0; i < resp.GetArraySize(); i++)
                {
                    if (resp[i]["transport_company"]("name") == _trans_comapny_name || _trans_comapny_name.empty())
                    {
                        ticket_meta tmp_meta;
                        tmp_meta.date = util_get_datestring(tmp_time);
                        tmp_meta.vehicle_number = resp[i]("driver_no");
                        tmp_meta.trans_company = std::to_string(i) + "_" + resp[i]["transport_company"]("name");
                        auto ticket_attach = resp[i]["attachment"];
                        for (auto j = 0; j < ticket_attach.GetArraySize(); j++)
                        {
                            auto url = ticket_attach[j]("attach");
                            if (url.find_first_of("http") != 0)
                            {
                                url.insert(0, "https://lng.hy3416.com");
                            }
                            tmp_meta.ticket_path.push_back(url);
                        }
                        all_ticket.push_back(tmp_meta);
                    }
                }

                return true;
            });
    }
    std::string req_serial = std::to_string(time(nullptr));
    std::string dir_fetch = "rm -rf /tmp/" + req_serial + "; mkdir /tmp/" + req_serial;
    system(dir_fetch.c_str());
    std::list<std::thread *> thread_pool;
    for (auto &itr : all_ticket)
    {
        auto tmp_t = new std::thread(
            [](ticket_meta _meta, const std::string &_req_se)
            {
                std::string dl_cmd = "wget -q -O '/tmp/" + _req_se + "/" + _meta.trans_company + "_" + _meta.date + "_" + _meta.vehicle_number;
                int i = 1;
                for (auto &path_itr : _meta.ticket_path)
                {
                    auto whole_dl_cmd = dl_cmd + "_" + std::to_string(i++) + ".jpg' " + path_itr;
                    system(whole_dl_cmd.c_str());
                }
            },
            itr, req_serial);
        thread_pool.push_back(tmp_t);
    }
    for (auto &itr:thread_pool)
    {
        itr->join();
        delete itr;
    }
    std::string zip_cmd = "zip -q /manage_dist/logo_res/" + req_serial + ".zip -r /tmp/" + req_serial;
    system(zip_cmd.c_str());
    zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "download_status", "/logo_res/" + req_serial + ".zip");
}