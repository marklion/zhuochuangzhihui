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
}

service user_management {
    list<user_info> get_all_user() throws (1:gen_exp e),
    user_info self_info(1:string ssid) throws (1:gen_exp e),
    string user_login(1:string phone, 2:string password) throws (1:gen_exp e),
}