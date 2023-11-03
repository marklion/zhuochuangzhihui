#if !defined(_RBAC_CENTER_H_)
#define _RBAC_CENTER_H_

#include "../../base/include.h"
class rbac_center_handler : public rbac_centerIf
{
public:
    rbac_center_handler();
    virtual bool has_permission(const std::string &token, const std::string &target_module, const std::string &target_resouce, const bool is_write);
    virtual bool add_role(const rbac_role &new_one);
    virtual bool del_role(const int64_t role_id);
    virtual void get_all_roles(std::vector<rbac_role> &_return);
    virtual bool add_user_to_role(const int64_t role_id, const int64_t user_id);
    virtual bool del_user_from_role(const int64_t role_id, const int64_t user_id);
    virtual bool add_role_module_permission(const int64_t role_id, const std::string &target_module);
    virtual bool del_role_module_permission(const int64_t role_id, const std::string &target_module);
    virtual bool add_role_resource_permission(const int64_t role_id, const std::string &target_resouce);
    virtual bool del_role_resource_permission(const int64_t role_id, const std::string &target_resouce);
    bool add_user(const rbac_user &new_one);
    virtual void del_user(const int64_t user_id);
    virtual void get_all_user(std::vector<rbac_user> &_return);
    virtual void get_all_permission(std::vector<std::string> &_return);
};

#endif // _RBAC_CENTER_H_
