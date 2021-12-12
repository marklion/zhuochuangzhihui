exception gen_exp {
    1:string msg,
}

struct hk_gate_device_ipconfig {
    1:string cam_ip;
    2:string led_ip;
}

struct device_gate_config {
    1:string name,
    2:string entry_id_reader_ip,
    3:string exit_id_reader_ip,
    4:hk_gate_device_ipconfig entry_config,
    5:hk_gate_device_ipconfig exit_config,
    6:string entry_qr_ip,
    7:string exit_qr_ip,
    8:bool entry_need_id,
    9:bool entry_need_qr,
    10:bool exit_need_id,
    11:bool exit_need_qr,
}

struct device_scale_config {
    1:string name,
    2:list<string> raster_ip,
    3:string scale_ip,
    4:string entry_printer_ip,
    5:string exit_printer_ip,
    6:string entry_id_reader_ip,
    7:string exit_id_reader_ip,
    8:hk_gate_device_ipconfig entry_config,
    9:hk_gate_device_ipconfig exit_config,
    10:string entry_qr_ip,
    11:string exit_qr_ip,
    12:bool need_id,
    13:bool need_qr,
}

struct device_config {
    1:list<device_gate_config> gate,
    2:list<device_scale_config> scale,
}

struct road_status {
    1:string coming_vehicle;
}

service system_management {
    bool reboot_system(1:string ssid) throws (1:gen_exp e),
    string current_version() throws (1:gen_exp e),
    device_config get_device_config(1:string ssid) throws (1:gen_exp e),
    bool edit_device_config(1:string ssid, 2:device_config config) throws (1:gen_exp e),
    bool raster_is_block(1:string raster_ip) throws (1:gen_exp e),
    bool print_content(1:string printer_ip, 2:string content, 3:string qr_code) throws (1:gen_exp e),
    string read_id_no(1:string id_reader_ip) throws (1:gen_exp e),
    bool ctrl_gate(1:string road_ip, 2:i64 cmd) throws (1:gen_exp e),
    bool ctrl_led(1:string led_ip, 2:string content) throws (1:gen_exp e),
    bool ctrl_voice(1:string led_ip, 2:string content) throws (1:gen_exp e),
    road_status get_road_status(1:string gate_code) throws (1:gen_exp e),
    double read_scale(1:string scale_ip) throws (1:gen_exp e),
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
    7:string admin_phone,
    8:string admin_password,
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
    9:bool in_white_list,
}


service vehicle_management {
    bool add_vehicle(1:string ssid, 2:vehicle_info vehicle) throws (1:gen_exp e),
    bool update_vehicle(1:string ssid, 2:vehicle_info vehicle) throws (1:gen_exp e),
    bool del_vehicle(1:string ssid, 2:i64 vehicle_id) throws (1:gen_exp e),
    list<vehicle_info> get_all_vehicle(1:string ssid) throws (1:gen_exp e),
}

struct order_status_info {
    1:string name,
    2:string timestamp,
    3:i64 step,
    4:string user_name,
}

struct vehicle_order_info {
    1:string order_number,
    2:string main_vehicle_number,
    3:string behind_vehicle_number,
    4:string driver_phone,
    5:string driver_name,
    6:string driver_id,
    7:i64 status,
    8:i64 id,
    9:string company_name,
    10:string stuff_name,
    11:list<order_status_info> status_details,
    12:double p_weight,
    13:double m_weight,
    14:string attachment,
}

struct gate_relate_info {
    1:string enter_time,
    2:string exit_time,
}

struct weight_relate_info {
    1:string p_time,
    2:string m_time,
    3:double p_weight,
    4:double m_weight,
}

struct vehicle_order_detail {
    1:vehicle_order_info basic_info,
    2:bool confirmed,
    3:bool registered,
    4:bool has_called,
}

service vehicle_order_center {
    list<vehicle_order_info> get_order_by_anchor(1:string ssid, 2:i64 anchor) throws (1:gen_exp e),
    gate_relate_info get_gate_info(1:string ssid, 2:i64 order_id) throws (1:gen_exp e),
    weight_relate_info get_weight_info(1:string ssid, 2:i64 order_id) throws (1:gen_exp e),
    bool create_vehicle_order(1:string ssid, 2:list<vehicle_order_info> order) throws (1:gen_exp e),
    bool confirm_vehicle_order(1:string ssid, 2:list<vehicle_order_info> order) throws (1:gen_exp e),
    bool cancel_vehicle_order(1:string ssid, 2:list<vehicle_order_info> order) throws (1:gen_exp e),
    vehicle_order_detail get_order_detail(1:string ssid, 2:string order_number) throws (1:gen_exp e),
    bool confirm_order_deliver(1:string ssid, 2:string order_number, 3:bool confirmed) throws (1:gen_exp e),
    bool update_vehicle_order(1:string ssid, 2:vehicle_order_info order) throws (1:gen_exp e),
    bool driver_check_in(1:i64 order_id, 2: bool is_cancel) throws (1:gen_exp e),
    vehicle_order_detail driver_get_order(1:string order_number) throws (1:gen_exp e),
    bool call_vehicle(1:string ssid, 2:i64 order_id, 3:bool is_cancel) throws (1:gen_exp e),
    list<vehicle_order_detail> get_registered_vehicle(1:string ssid) throws (1:gen_exp e),
    bool manual_set_p_weight(1:string ssid, 2:i64 order_id, 3:double weight) throws (1:gen_exp e),
    bool manual_set_m_weight(1:string ssid, 2:i64 order_id, 3:double weight) throws (1:gen_exp e),
    bool manual_close(1:string ssid, 2:i64 order_id) throws (1:gen_exp e),
}

service open_api {
    bool vehicle_come(1:string plateNo, 2:string road) throws (1:gen_exp e),
    bool vehicle_leave(1:string plateNo, 2:string road) throws (1:gen_exp e),
    bool external_trigger_gate_vehicle(1:string road_ip, 2:string plate_no) throws (1:gen_exp e),
    bool external_trigger_gate_id(1:string id_reader_ip, 2:string id_no, 3:string road_ip) throws (1:gen_exp e),
    bool external_trigger_scale_vehicle(1:string road_ip, 2:string plate_no, 3:string scale_name) throws (1:gen_exp e),
    bool external_trigger_scale_id(1:string id_reader_ip, 2:string id_no, 3:string scale_name) throws (1:gen_exp e),
}