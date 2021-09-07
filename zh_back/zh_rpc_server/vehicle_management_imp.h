#if !defined(_VEHICLE_MANAGEMENT_IMP_H_)
#define _VEHICLE_MANAGEMENT_IMP_H_

#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_management.h"

class vehicle_management_handler : public vehicle_managementIf {
private:
    static vehicle_management_handler *m_inst;
    vehicle_management_handler() {}
public:
    static vehicle_management_handler *get_inst() {
        if (nullptr == m_inst)
        {
            m_inst = new vehicle_management_handler();
        }
        return m_inst;
    }
    virtual bool add_vehicle(const std::string &ssid, const vehicle_info &vehicle);
    virtual bool update_vehicle(const std::string &ssid, const vehicle_info &vehicle);
    virtual bool del_vehicle(const std::string &ssid, const int64_t vehicle_id);
    virtual void get_all_vehicle(std::vector<vehicle_info> &_return, const std::string &ssid);
};

#endif // _VEHICLE_MANAGEMENT_IMP_H_
