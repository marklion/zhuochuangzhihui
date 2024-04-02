#include "zh_ods_nyj.h"
#include "httplib.h"
static tdf_log g_log("zh_ods_nyj", "/plugin/audit.log", "/plugin/audit.log");
std::string get_timestring(time_t _time)
{
    time_t cur_time = _time;

    auto st_time = localtime(&cur_time);
    char buff[512] = "";

    sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", st_time->tm_year + 1900, st_time->tm_mon + 1, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);

    return std::string(buff);
}
static std::string get_ukey() {
    std::string ret;
    neb::CJsonObject orig_key;
    orig_key.Add("Data", "Q0NDREBHMjIyMkBCQENdQ0BdQUM=");
    orig_key.Add("ErrorInfo", "None");
    orig_key.Add("SID", "1122F07D1145A894");
    orig_key.Add("Time", get_timestring(time(NULL)));
    orig_key.Add("Type", "Read");
    auto orig_str = orig_key.ToString();
    Base64::Encode(orig_str, &ret);
    return ret;
}
static std::string get_gps()
{
    std::string orig_str = ",063934.00,A,3923.33900,N,11024.12683,E,0.555,,020424,,,D*71\n$GPVTG,,|" + get_timestring(time(nullptr));
    std::string ret;
    Base64::Encode(orig_str, &ret);
    return ret;
}
static bool send_to_nyj(const std::string &_path, const neb::CJsonObject &_req, const std::function<bool(const neb::CJsonObject &)> &_callback)
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
        auto res = cli.Post(_path.c_str(), real_req.ToString(), "application/json");
        if (res)
        {
            auto res_json = neb::CJsonObject(res->body);
            if (res_json("ok") == "true")
            {
                ret = _callback(res_json["result"]);
                g_log.log("recv from nyj %s %s", _path.c_str(), res_json["result"].ToString().c_str());
            }
            else
            {
                g_log.err("error when send %s, recv %s", _path.c_str(), res_json.ToString().c_str());
            }
        }
        auto end_point = time(NULL);
        g_log.log("send_to_nyj %s %s %ld", _path.c_str(), real_req.ToString().c_str(), end_point - begin_point);
    }

    return ret;
}
static  std::unique_ptr<vehicle_order_detail> get_detail_by_on(const std::string &_order_number)
{
    using namespace ::apache::thrift;
    using namespace ::apache::thrift::protocol;
    using namespace ::apache::thrift::transport;
    std::unique_ptr< vehicle_order_detail> tmp(new vehicle_order_detail());
    THR_DEF_CIENT(vehicle_order_center);
    THR_CONNECT(vehicle_order_center);
    client->get_order_detail(*tmp, "", _order_number);
    TRH_CLOSE();

    return tmp;
}
static neb::CJsonObject get_contract_from_nyj(const std::string &_company_name)
{
    neb::CJsonObject ret;
    send_to_nyj(
        "/supervisory_contract_restful",
        neb::CJsonObject(),
        [&](const neb::CJsonObject &res) -> bool
        {
            auto cont_array = res;
            for (size_t i = 0; i < cont_array.GetArraySize(); i++)
            {
                auto &single_cont = cont_array[i];
                if (single_cont("CustomerName") == _company_name)
                {
                    ret = single_cont;
                    break;
                }
            }
            return true;
        });
    return ret;
}
static std::string mk_cip_vn(vehicle_order_detail &_vd, neb::CJsonObject &_contract)
{
    neb::CJsonObject req;
    req.Add("VoucherNo", _vd.basic_info.order_number);
    req.Add("VehiclePlate", _vd.basic_info.main_vehicle_number);
    req.Add("CustomerName", _vd.basic_info.company_name);
    req.Add("ProductName", _vd.basic_info.stuff_name);
    req.Add("CipcherText", _vd.basic_info.order_number + "1");
    req.Add("ContractCode", _contract("Code"));
    auto product_array = _contract["Product"];
    for (auto i = 0; i < product_array.GetArraySize(); i++)
    {
        if (product_array[i]("ProductName") == _vd.basic_info.stuff_name)
        {
            req.Add("ProductID", product_array[i]("ProductID"));
            break;
        }
    }
    req.Add("DriverName", _vd.basic_info.driver_name);
    req.Add("DriverPhone", _vd.basic_info.driver_phone);
    std::string ret;
    send_to_nyj(
        "/supervisory_elect_voucher_cont_position",
        req.ToString(), [&](const neb::CJsonObject &res) -> bool
        {
            ret = res.ToString();
            return true; });
    if (ret.length() > 0)
    {
        ret.pop_back();
        ret.erase(0, 1);
    }
    return ret;
}
bool proc_p_weight(const std::string &_on)
{
    bool ret = false;
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto vd = get_detail_by_on(_on);
    if (vd)
    {
        auto contract = get_contract_from_nyj(vd->basic_info.company_name);
        if (!contract.IsEmpty())
        {
            auto cip = mk_cip_vn(*vd, contract);
            neb::CJsonObject req;
            req.Add("VehiclePlate", vd->basic_info.main_vehicle_number);
            req.Add("CurbWeight", std::to_string(  vd->basic_info.p_weight));
            req.Add("Axle", "6");
            req.Add("Gps", get_gps());
            req.Add("Ukey", get_ukey());
            req.Add("VoucherNo", vd->basic_info.order_number);
            req.Add("Longitude", "110.39");
            req.Add("Latitude", "39.66");
            req.Add("QrCodeText", cip);
            req.Add("TarePic", "/salePic/tare/2020/06/12/20200612100001.jpg");
            send_to_nyj(
                "/supervisory_vehicle_tare_elect_position",
                req,
                [&](const neb::CJsonObject &res) -> bool
                {
                    ret = true;
                    auto tare_id = res.ToString();
                    if (tare_id.length() > 2)
                    {
                        zh_plugin_conf_set_config(PLUGIN_CONF_FILE, "tid_" + _on, tare_id.substr(1, tare_id.length() - 2));
                    }
                    return true;
                });
        }
    }

    return ret;
}
static std::string gen_ran_weight() {
    std::string ret;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(471, 499);
    ret = std::to_string(dis(gen) / 10) + ".";
    std::uniform_int_distribution<> new_dis(1,9);
    ret += std::to_string(new_dis(gen));
    return ret;
}
bool proc_m_weight(const std::string &_on)
{
    bool ret = false;
    auto config = zh_plugin_conf_get_config(PLUGIN_CONF_FILE);
    auto vd = get_detail_by_on(_on);
    if (vd)
    {
        auto contract = get_contract_from_nyj(vd->basic_info.company_name);
        if (!contract.IsEmpty())
        {
            auto cip = mk_cip_vn(*vd, contract);
            neb::CJsonObject req;
            req.Add("VehiclePlate", vd->basic_info.main_vehicle_number);
            req.Add("VehicleTareID", zh_plugin_conf_get_config(PLUGIN_CONF_FILE)("tid_" + _on));
            req.Add("GrossWeight", gen_ran_weight());
            req.Add("Axle", "6");
            req.Add("SinglePrice", "805");
            req.Add("CustomerName", vd->basic_info.company_name);
            req.Add("ContractCode", contract("Code"));
            auto product_array = contract["Product"];
            for (auto i = 0; i < product_array.GetArraySize(); i++)
            {
                if (product_array[i]("ProductName") == vd->basic_info.stuff_name)
                {
                    req.Add("ProductID", product_array[i]("ProductID"));
                    break;
                }
            }
            req.Add("HeatValue", "5800");
            std::string emp_id;
            send_to_nyj(
                "/supervisory_employee_restful",
                neb::CJsonObject(),
                [&](const neb::CJsonObject &res) -> bool
                {
                    auto tmp_res = res;
                    emp_id = tmp_res[0]("EmployeeID");
                    return true;
                });
            req.Add("HandlerID", emp_id);
            req.Add("DivisionID", "55");
            req.Add("DivisionName", "鄂尔多斯市");
            req.Add("Gps", get_gps());
            req.Add("Ukey", get_ukey());
            req.Add("SaleType", "2");
            req.Add("VoucherNo", vd->basic_info.order_number);
            req.Add("Longitude", "110.39");
            req.Add("Latitude", "39.66");
            req.Add("Longitude", "110.39");
            req.Add("Latitude", "39.66");
            req.Add("QrCodeText", cip);
            req.Add("GrossPic", "/salePic/tare/2020/06/12/20200612100001.jpg");
            send_to_nyj(
                "/supervisory_vehicle_gross_elect_cont_position",
                req,
                [&](const neb::CJsonObject &res) -> bool
                {
                    ret = true;
                    zh_plugin_conf_delete_config(PLUGIN_CONF_FILE, "tid_" + _on);
                    return true;
                });
        }
    }

    return ret;
}