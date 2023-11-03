exception gen_exp {
    1:string msg,
}

struct stuff_config {
    1:i64 id,
    2:string stuff_name,
}

service config_management{
    list<stuff_config> get_stuff_config() throws (1:gen_exp e),
    bool add_stuff_config(1:stuff_config new_one) throws (1:gen_exp e),
    bool del_stuff_config(1:i64 id) throws (1:gen_exp e),
}

struct rbac_user {
    1:string name,
    2:i64 id,
    3:string phone,
    4:list<string> role_name,
    5:string md5_password,
}

struct rbac_role{
    1:string role_name,
    2:i64 id,
    3:list<rbac_user> all_user,
    4:list<string> author_modules,
    5:list<string> author_resouces,
    6:bool read_only,
}

service rbac_center{
    bool has_permission(1:string token, 2:string target_module, 3:string target_resouce, 4:bool is_write) throws (1:gen_exp e),
    bool add_role(1:rbac_role new_one) throws (1:gen_exp e),
    bool del_role(1:i64 role_id) throws (1:gen_exp e),
    list<rbac_role> get_all_roles() throws (1:gen_exp e),
    bool add_user_to_role(1:i64 role_id, 2:i64 user_id) throws (1:gen_exp e),
    bool del_user_from_role(1:i64 role_id, 2:i64 user_id) throws (1:gen_exp e),
    bool add_role_module_permission(1:i64 role_id, 2:string target_module ) throws (1:gen_exp e),
    bool del_role_module_permission(1:i64 role_id, 2:string target_module ) throws (1:gen_exp e),
    bool add_role_resource_permission(1:i64 role_id, 2:string target_resouce) throws (1:gen_exp e),
    bool del_role_resource_permission(1:i64 role_id, 2:string target_resouce) throws (1:gen_exp e),
    bool add_user(1:rbac_user new_one) throws (1:gen_exp e),
    void del_user(1:i64 user_id) throws (1:gen_exp e),
    list<rbac_user> get_all_user() throws (1:gen_exp e),
    list<string> get_all_permission() throws (1:gen_exp e),
}