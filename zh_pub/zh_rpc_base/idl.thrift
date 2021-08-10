exception gen_exp {
    1:string msg,
}

service system_management {
    bool reboot_system(1:string ssid) throws (1:gen_exp e),
    string current_version() throws (1:gen_exp e),
}