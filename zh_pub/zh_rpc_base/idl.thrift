exception gen_exp {
    1:string msg,
}

service system_management {
    bool reboot_system(1:string ssid) throws (1:gen_exp e),
    string current_version() throws (1:gen_exp e),
}

struct user_info {
    1:string name,
    2:string phone,
    3:string password,
    4:i64 id,
    5:i64 permission,
    6:string permission_name,
    7:bool need_change_password,
}

struct user_permission {
    1:string name,
    2:string description,
    3:i64 id,
}

service user_management {
    list<user_permission> get_all_permissions(1:string ssid) throws (1:gen_exp e),
    list<user_info> get_all_user(1:string ssid) throws (1:gen_exp e),
    user_info self_info(1:string ssid) throws (1:gen_exp e),
    string user_login(1:string phone, 2:string password) throws (1:gen_exp e),
    void user_logoff(1:string ssid) throws (1:gen_exp e),
    bool add_user(1:string ssid, 2:user_info new_user) throws (1:gen_exp e),
    bool del_user(1:string ssid, 2:i64 user_id) throws (1:gen_exp e),
    bool reset_user(1:string ssid, 2:i64 user_id, 3:string password) throws (1:gen_exp e),
    bool change_user_password(1:string ssid, 2:string password) throws (1:gen_exp e),
}

struct contract_info {
    1:string name,
    2:string date,
    3:bool is_sale,
    4:string attachment,
    5:i64 id,
    6:string code,
}

service contract_management {
    bool add_contract(1:string ssid, 2:contract_info contract) throws (1:gen_exp e),
    bool del_contract(1:string ssid, 2:i64 contract_id) throws (1:gen_exp e),
    bool update_contract(1:string ssid, 2:contract_info contract) throws (1:gen_exp e),
    list<contract_info> get_all_contract(1:string ssid) throws (1:gen_exp e),
}