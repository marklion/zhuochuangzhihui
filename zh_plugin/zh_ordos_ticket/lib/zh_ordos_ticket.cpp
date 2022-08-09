#include "zh_ordos_ticket.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include "httplib.h"
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
#define PLUGIN_CONF_FILE "/plugin/zh_ordos_ticket/conf/plugin.json"

static tdf_log g_log("ordos_ticket_audit", "/plugin/audit.log", "/plugin/audit.log");
void zh_ordos_ticket_order_finish(const std::string &_msg)
{
    std::cout << "proc order finish,msg:" << _msg << std::endl;
}

static std::string zh_ordos_req_with_token(const std::string &_url, const std::string &_req = "")
{
    std::string ret;
    std::fstream in_file(PLUGIN_CONF_FILE, std::ios::in);
    std::istreambuf_iterator<char> beg(in_file), end;
    std::string config_string(beg, end);
    neb::CJsonObject config(config_string);
    httplib::Client cli(config("remote_url"));
    cli.set_bearer_token_auth(config("token").c_str());
    httplib::Headers ref = {{"Referer", config("remote_url") + "/view/access_process/gross_weight"}, {"Origin", config("remote_url")}};
    cli.set_default_headers(ref);

    g_log.log("send req to ordos_ticket:%s,req:%s", _url.c_str(), _req.c_str());
    auto res = cli.Get(_url.c_str());
    if (_req.length() > 0)
    {
        res = cli.Post(_url.c_str(), _req, "application/json");
    }
    if (res)
    {
        neb::CJsonObject res_body(res->body);
        if (res_body("ok") == "true")
        {
            ret = res_body["result"].ToString();
        }
        else
        {
            std::cerr << res->body << std::endl;
        }
        g_log.log("recv from ordos_ticket:%s", res_body.ToFormattedString().c_str());
        auto token = res->get_header_value("Token");
        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "token", token);
    }
    else
    {
        std::cerr << res.error() << std::endl;
    }
    return ret;
}
std::string zh_ordos_ticket_get_TIDs()
{
    neb::CJsonObject all_tids(zh_ordos_req_with_token("/getWarehouses"));
    return all_tids["warehouses"].ToString();
}

std::string zh_ordos_ticket_get_verify_code()
{
    neb::CJsonObject config(zh_plugin_conf_get_config(PLUGIN_CONF_FILE));
    httplib::Client cli(config("remote_url"));
    auto res = cli.Get("/getCaptcha");
    if (!res)
    {
        return "";
    }
    auto res_body = res->body;
    neb::CJsonObject res_json(res_body);
    auto png_body = res_json("captcha");
    png_body = png_body.substr(22, png_body.length() - 22);
    zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "login_key", res_json("idKey"));
    return png_body;
}
static void zh_ordos_fetch_basic()
{
    auto res = zh_ordos_req_with_token("/vehicles/test_args");
    auto basic_config = neb::CJsonObject(res);
    basic_config.Delete("commonlyDestinations");
    basic_config.AddEmptySubArray("commonlyDestinations");
    auto all_location = zh_ordos_req_with_token("/getFilterLocations");
    auto location_config = neb::CJsonObject(all_location)["locations"];
    for (auto i = 0; i < location_config.GetArraySize(); i++)
    {
        neb::CJsonObject tmp;
        tmp.Add("DestinationID", location_config[i]("ID"));
        tmp.Add("DestinationName", location_config[i]("Name"));
        basic_config["commonlyDestinations"].Add(tmp);
    }
    zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "basic_data", basic_config);
}
bool zh_ordos_ticket_login(const std::string &_username, const std::string &_password, const std::string &_verify_code)
{
    bool ret = false;
    neb::CJsonObject login_req;
    neb::CJsonObject config(zh_plugin_conf_get_config(PLUGIN_CONF_FILE));

    login_req.Add("Captcha", _verify_code);
    login_req.Add("Key", config("login_key"));
    login_req.Add("LoginName", _username);
    login_req.Add("Password", _password);

    httplib::Client cli(config("remote_url"));
    auto res = cli.Post("/login/auth", login_req.ToString(), "application/json");
    if (res)
    {
        neb::CJsonObject res_body(res->body);
        if (res_body("ok") == "true")
        {
            zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "OID", res_body("organization"));
            zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "token", res_body("token"));
            zh_ordos_fetch_basic();
            ret = true;
        }
        else
        {
            std::cerr << "登录失败" << res_body.ToString() << std::endl;
        }
    }
    else
    {
        std::cerr << "登录失败" << res.error() << std::endl;
    }
    return ret;
}

bool zh_ordos_ticket_print_ticket(const std::string &_msg)
{
    bool ret = false;
    neb::CJsonObject print_req(_msg);
    // for debug
    //  auto orig_url = zh_ordos_ticket_get_config()("remote_url");
    //  zh_ordos_set_config("remote_url", "https://www.fastmock.site/mock/34e68b17ea9b4e8b3d37f02809fed8af/thirdParty");
    auto gross_info = neb::CJsonObject(_msg);
    neb::CJsonObject curb_weight;

    curb_weight.Add("VehicleNum", gross_info("VehicleNum"));
    curb_weight.Add("CurbWeight", gross_info("EmptyWeight"));
    curb_weight.Add("Axes", gross_info("Axes"));

    auto additional_config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE)["additional_config"];
    for (auto i = 0; i < additional_config.GetArraySize(); i++)
    {
        auto additional_key = additional_config[i]("key");
        auto additional_value = additional_config[i]("value");
        gross_info.Add(additional_key, additional_value);
        curb_weight.Add(additional_key, additional_value);
    }
    gross_info.Delete("ClearInventory");
    gross_info.Add("ClearInventory", "0");
    gross_info.Delete("GuarantHeat");
    gross_info.Add("GuarantHeat", "0");
    gross_info.Add("GPS", zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("gps"));
    curb_weight.Add("GPS", zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("gps"));

    zh_ordos_req_with_token("/vehicles/curb_weight", curb_weight.ToString());
    zh_ordos_req_with_token("/vehicles/supervisory_gross_weight", gross_info.ToString());
    // for debug
    //  zh_ordos_set_config("remote_url", orig_url);
    auto ticket_number = print_req("WarehouseDetail");

    // for debug
    //  auto prev_number = atol(ticket_number.substr(1, ticket_number.length() - 1).c_str()) - 1;
    //  ticket_number = ticket_number.substr(0, 1) + std::to_string(prev_number);
    neb::CJsonObject config(zh_plugin_conf_get_config(PLUGIN_CONF_FILE));

    std::string set_print_cmd = "lpadmin -x rp;lpadmin -p rp -E -v '" + config("device_uri") + "' && lpadmin -d rp";
    system(set_print_cmd.c_str());

    auto oid = config("OID");
    auto req = "/viewAndPrintTicket?Num=" + ticket_number + "&organizationID=" + oid;
    std::string cmd = "wget '" + config("remote_url") + req + "' -q -O- | lp -s -";
    if (0 == system(cmd.c_str()))
    {
        ret = true;
    }
    return ret;
}
static std::string zh_ordos_ticket_find_basic_data(const std::string &_type, const std::string &_name)
{
    std::string ret;
    auto basic_data = zh_plugin_conf_get_config(PLUGIN_CONF_FILE)["basic_data"];
    auto focus_data = basic_data[_type];
    for (auto i = 0; i < focus_data.GetArraySize(); i++)
    {
        if (focus_data[i]("Name") == _name)
        {
            ret = focus_data[i]("ID");
            break;
        }
        if (focus_data[i]("DestinationName") == _name)
        {
            ret = focus_data[i]("DestinationID");
            break;
        }
    }

    return ret;
}
bool zh_ordos_ticket_proc_finish(const std::string &_msg)
{
    bool ret = false;
    zh_ordos_fetch_basic();
    auto tid = zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("TID");
    auto res = neb::CJsonObject(zh_ordos_req_with_token("/getNextTicket?ID=" + tid));
    auto ticket_number = res("ticketNo");

    neb::CJsonObject orig_req(_msg);
    neb::CJsonObject req;

    req.Add("VehicleNum", orig_req("main_vehicle_number"));
    req.Add("EmptyWeight", orig_req("p_weight"));
    req.Add("GrossWeight", orig_req("m_weight"));
    req.Add("Axes", orig_req("axes"));
    req.Add("CurbWeight", orig_req("j_weight"));
    req.Add("CurbWeightDateTime", orig_req("finish_date"));
    req.Add("ProductID", zh_ordos_ticket_find_basic_data("products", orig_req("stuff_name")));
    auto price = zh_ordos_req_with_token("/vehicles/get_product_price?productID=" + req("ProductID"));
    req.Add("SinglePrice", price.substr(1, price.length() - 2));
    req.Add("CustomerID", zh_ordos_ticket_find_basic_data("customers", orig_req("company_name")));
    req.Add("DivisionID", zh_ordos_ticket_find_basic_data("commonlyDestinations", orig_req("company_address")));
    req.Add("DivisionName", orig_req("company_address"));
    req.Add("DivisionDetail", "");
    req.Add("WarehouseDetail", ticket_number);
    req.Add("ClearInventory", "0");
    req.Add("GuarantHeat", "0");
    req.Add("Category", zh_ordos_ticket_find_basic_data("categorys", orig_req("use_for")));

    ret = zh_ordos_ticket_print_ticket(req.ToString());

    return ret;
}

bool zh_ordos_ticket_init()
{
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    config.Delete("remote_url");
    zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "remote_url", "http://58.18.38.116:8811");
    return true;
}

void zh_ordos_ticket_refresh()
{
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    if (config("token").length() > 0)
    {
        zh_ordos_fetch_basic();
    }
}

bool zh_ordos_ticket_p_weight(const std::string &_vehicle_number, const std::string &_p_weight)
{
    bool ret = false;

    neb::CJsonObject curb_weight;

    curb_weight.Add("VehicleNum", _vehicle_number);
    curb_weight.Add("CurbWeight", _p_weight);
    curb_weight.Add("Axes", "6");
    curb_weight.Add("GPS", zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("gps"));

    if (zh_ordos_req_with_token("/vehicles/curb_weight", curb_weight.ToString()).length() > 0)
    {
        ret = true;
    }

    return ret;
}