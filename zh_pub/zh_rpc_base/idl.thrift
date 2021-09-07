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

struct stuff_info {
    1:string name,
    2:double inventory,
    3:string unit,
    4:i64 id,
}

service stuff_management {
    bool add_stuff(1:string ssid, 2:stuff_info stuff) throws (1:gen_exp e),
    bool update_stuff(1:string ssid, 2:stuff_info stuff) throws (1:gen_exp e),
    bool del_stuff(1:string ssid, 2:i64 id) throws (1:gen_exp e),
    list<stuff_info> get_all_stuff(1:string ssid) throws (1:gen_exp e),
}

struct vehicle_info {
    1:string main_vehicle_number,
    2:string behind_vehicle_number,
    3:string driver_name,
    4:string driver_phone,
    5:string driver_id,
    6:string company_name,
    7:i64 id,
    8:string group_name,
}


service vehicle_management {
    bool add_vehicle(1:string ssid, 2:vehicle_info vehicle) throws (1:gen_exp e),
    bool update_vehicle(1:string ssid, 2:vehicle_info vehicle) throws (1:gen_exp e),
    bool del_vehicle(1:string ssid, 2:i64 vehicle_id) throws (1:gen_exp e),
    list<vehicle_info> get_all_vehicle(1:string ssid) throws (1:gen_exp e),
}
