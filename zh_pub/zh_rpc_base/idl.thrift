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
    12:string entry_nvr_ip,
    13:string exit_nvr_ip,
    14:i64 entry_channel,
    15:i64 exit_channel,
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
    14:string scale_brand,
    15:string entry_nvr_ip,
    16:string exit_nvr_ip,
    17:i64 entry_channel,
    18:i64 exit_channel,
    19:double coefficient = 1,
}

struct device_config {
    1:list<device_gate_config> gate,
    2:list<device_scale_config> scale,
    3:bool auto_order,
}

struct road_status {
    1:string coming_vehicle,
}

struct device_health {
    1:string name,
    2:i64 entry_cam = -1,
    3:i64 exit_cam = -1,
    4:i64 entry_led = -1,
    5:i64 exit_led = -1,
    6:i64 entry_id = -1,
    7:i64 exit_id = -1,
    8:i64 entry_qr = -1,
    9:i64 exit_qr = -1,
    10:i64 raster1 = -1,
    11:i64 raster2 = -1,
    12:i64 entry_printer = -1,
    13:i64 exit_printer = -1,
    14:i64 scale = -1,
}

service system_management {
    bool reboot_system(1:string ssid) throws (1:gen_exp e),
    string current_version() throws (1:gen_exp e),
    device_config get_device_config(1:string ssid) throws (1:gen_exp e),
    bool edit_device_config(1:string ssid, 2:device_config config) throws (1:gen_exp e),
    bool raster_is_block(1:string raster_ip) throws (1:gen_exp e),
    bool print_content(1:string printer_ip, 2:string content, 3:string qr_code) throws (1:gen_exp e),
    string read_id_no(1:string id_reader_ip) throws (1:gen_exp e),
    string read_qr(1:string id_reader_ip) throws (1:gen_exp e),
    bool ctrl_gate(1:string road_ip, 2:i64 cmd) throws (1:gen_exp e),
    bool led_cast_welcome(1:string led_ip) throws (1:gen_exp e),
    road_status get_road_status(1:string gate_code) throws (1:gen_exp e),
    double read_scale(1:string scale_ip) throws (1:gen_exp e),
    void run_update(1:string ssid, 2:string pack_path) throws (1:gen_exp e),
    string get_domain_name() throws (1:gen_exp e),
    string get_oem_name() throws (1:gen_exp e),
    list<string> get_all_scale_brand() throws (1:gen_exp e),
    list<device_health> get_device_health(1:string ssid) throws (1:gen_exp e),
    void trigger_cap(1:string ssid, 2:string cam_ip) throws (1:gen_exp e),
    bool is_auto_confirm(1:string ssid) throws (1:gen_exp e),
    void set_auto_confirm(1:string ssid, 2:bool auto_set) throws (1:gen_exp e),
    void manual_confirm_scale(1:string ssid, 2:string scale_name) throws (1:gen_exp e),
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
    9:string company_address,
    10:double balance,
    11:double credit,
    12:list<string> follow_stuffs,
}

struct number_change_point{
    1:string timestamp,
    2:string reason,
    3:double change_value,
    4:double new_value,
}

service contract_management {
    bool add_contract(1:string ssid, 2:contract_info contract) throws (1:gen_exp e),
    bool del_contract(1:string ssid, 2:i64 contract_id) throws (1:gen_exp e),
    bool update_contract(1:string ssid, 2:contract_info contract) throws (1:gen_exp e),
    list<contract_info> get_all_contract(1:string ssid) throws (1:gen_exp e),
    contract_info get_contract(1:string ssid, 2:string company_name) throws (1:gen_exp e),
    list<number_change_point> get_history(1:string ssid, 2:string company_name, 3:i64 count) throws (1:gen_exp e),
    bool change_balance(1:string ssid, 2:string company_name, 3:double new_value, 4:string reason) throws (1:gen_exp e),
}

struct stuff_info {
    1:string name,
    2:double inventory,
    3:string unit,
    4:i64 id,
    5:bool need_enter_weight,
    6:double price,
    10:double expect_weight,
    11:bool need_manual_scale,
}

struct stuff_change_point {
    1:string date,
    2:double inventory,
}

struct stuff_history {
    1:stuff_info stuff,
    2:list<stuff_change_point> change_point,
}

service stuff_management {
    bool add_stuff(1:string ssid, 2:stuff_info stuff) throws (1:gen_exp e),
    bool update_stuff(1:string ssid, 2:stuff_info stuff) throws (1:gen_exp e),
    bool del_stuff(1:string ssid, 2:i64 id) throws (1:gen_exp e),
    list<stuff_info> get_all_stuff(1:string ssid, 2:string user_name) throws (1:gen_exp e),
    stuff_info get_stuff(1:string ssid, 2:string stuff_name) throws (1:gen_exp e),
    list<stuff_history> get_change_points_for_range(1:string ssid, 2:string start_date, 3:string end_date) throws (1:gen_exp e),
    string get_last_active(1:string ssid) throws (1:gen_exp e),
    list<number_change_point> get_history(1:string ssid, 2:string stuff_name, 3:i64 count) throws (1:gen_exp e),
    bool change_price(1:string ssid, 2:string stuff_name, 3:double new_value) throws (1:gen_exp e),
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
    10:double max_count = 35,
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
    15:string enter_weight_attachment,
    16:double enter_weight,
    17:bool need_enter_weight,
    18:string company_address,
    19:string use_for,
    20:string balance_warn,
    21:double max_count,
    22:string end_time,
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
    5:string enter_nvr_ip,
    6:string exit_nvr_ip,
    7:string p_nvr_ip1,
    8:string p_nvr_ip2,
    9:string m_nvr_ip1,
    10:string m_nvr_ip2,
    11:string enter_time,
    12:string exit_time,
    13:string p_time,
    14:string m_time,
}

struct vehicle_order_statistics {
    1:i64 total,
    2:i64 created,
    3:i64 confirmed,
    4:i64 entered,
    5:i64 first_weight,
    6:i64 second_weight,
}

struct driver_self_order {
    1:i64 id,
    2:string main_vehicle_number,
    3:string driver_name,
    4:string driver_phone,
    5:string driver_id,
    6:string stuff_name,
    7:string belong_user_name,
}

service vehicle_order_center {
    list<vehicle_order_info> get_order_by_anchor(1:string ssid, 2:i64 anchor, 3:string status_name, 4:string enter_date) throws (1:gen_exp e),
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
    vehicle_order_statistics get_order_statistics(1:string ssid) throws (1:gen_exp e),
    bool upload_enter_weight_attachment(1:i64 order_id, 2:string attachment, 3:double enter_weight) throws (1:gen_exp e),
    bool print_weight_ticket(1:string ssid, 2:i64 order_id, 3:string scale_name) throws (1:gen_exp e),
    string check_price_balance(1:string ssid, 2:list<vehicle_order_info> order) throws (1:gen_exp e),
    bool create_driver_self_order(1:driver_self_order order) throws (1:gen_exp e),
    bool confirm_driver_self_order(1:string ssid, 2:i64 order_id, 3:bool continue_order) throws (1:gen_exp e),
    bool cancel_driver_self_order(1:string ssid, 2:i64 order_id) throws (1:gen_exp e),
    list<driver_self_order> get_all_self_order(1:string ssid) throws (1:gen_exp e),
    driver_self_order get_self_order_by_phone(1:string driver_phone) throws (1:gen_exp e),
}

service open_api {
    bool vehicle_come(1:string plateNo, 2:string road) throws (1:gen_exp e),
    bool vehicle_leave(1:string plateNo, 2:string road) throws (1:gen_exp e),
    bool external_trigger_gate_vehicle(1:string road_ip, 2:string plate_no) throws (1:gen_exp e),
    bool external_trigger_gate_id(1:string id_reader_ip, 2:string id_no, 3:string road_ip) throws (1:gen_exp e),
    bool external_trigger_scale_vehicle(1:string road_ip, 2:string plate_no, 3:string scale_name) throws (1:gen_exp e),
    bool external_trigger_scale_id(1:string id_reader_ip, 2:string id_no, 3:string scale_name) throws (1:gen_exp e),
    bool external_trigger_gate_qr(1:string road_ip, 2:string qr_code) throws (1:gen_exp e),
    bool external_trigger_scale_qr(1:string scale_ip, 2:string qr_code, 3:string scale_name) throws (1:gen_exp e),
    string get_video(1:string nvr_ip, 2:i64 channel_id, 3:string start_time, 4:string stop_time) throws (1:gen_exp e),
}

service plugin_management {
    string run_plugin_cmd(1:string ssid, 2:string plugin_name, 3:string cmd) throws (1:gen_exp e),
    bool install_plugin(1:string ssid, 2:string plugin_name, 3:string file_name) throws (1:gen_exp e),
    void uninstall_plugin(1:string ssid, 2:string plugin_name) throws (1:gen_exp e),
    list<string> get_installed_plugins(1:string ssid) throws (1:gen_exp e),
}