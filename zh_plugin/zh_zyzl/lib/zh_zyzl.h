#if !defined(_ZH_ZYZL_H_)
#define _ZH_ZYZL_H_
#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../../zh_pub/zh_cpp_pub/Base64.h"
#include "../../../zh_back/zh_tdf/tdf_include.h"

struct zy_sync_plan_data{
    std::string planNo;
    std::string behindPlateNo;
    std::string driverName;
    std::string driverPhone;
    std::string driverID;
    std::string useFor;
    std::string deliverAddress;
    std::string arriveDate;
    std::string customerName;
    std::string stuffName;
    std::string order_number;
    bool operator==(const zy_sync_plan_data &_other) const
    {
        bool ret = false;

        if (planNo == _other.planNo || behindPlateNo == _other.behindPlateNo)
        {
            ret = true;
        }

        return ret;
    }
};

zy_sync_plan_data json_to_struct_plan(const neb::CJsonObject &_json);
bool ZH_ZYZL_call_vehicle(const std::string &_plate);
bool ZH_ZYZL_push_p(const std::string &_plate);
bool ZH_ZYZL_push_weight(const std::string &_plate, const std::string &_p_time, const std::string &_m_time, double p_weight, double m_weight, double j_weight, const std::string &_ticket_no, const std::string &_seal_no);
void ZH_ZYZL_sync_plans(const std::list<zy_sync_plan_data> &_plan_date);
std::string ZH_ZYZL_pull_ticket(const std::string &_plate);


#endif // _ZH_ZYZL_H_
