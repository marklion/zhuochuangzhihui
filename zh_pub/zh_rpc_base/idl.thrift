exception gen_exp {
    1:string msg,
}

struct prompt_image_info{
    1:i64 id,
    2:string attachment_path,
}

struct company_address_info{
    1:double x,
    2:double y,
    3:double distance,
}

struct register_config_info {
    1:bool enabled,
    2:i64 pass_time,
    3:i64 check_in_time,
    4:i64 leave_limit,
}

struct device_status {
    1:string name,
    2:bool enter_gate_is_close,
    3:bool exit_gate_is_close,
    4:string cur_status,
    5:double cur_weight,
    6:bool is_scale,
}

service system_management {
    bool reboot_system(1:string ssid) throws (1:gen_exp e),
    string current_version() throws (1:gen_exp e),
    void run_update(1:string ssid, 2:string pack_path) throws (1:gen_exp e),
    string get_domain_name() throws (1:gen_exp e),
    string get_oem_name() throws (1:gen_exp e),
    bool is_auto_confirm(1:string ssid) throws (1:gen_exp e),
    void set_auto_confirm(1:string ssid, 2:bool auto_set) throws (1:gen_exp e),
    bool upload_prompt_image(1:string ssid, 2:string attachment) throws (1:gen_exp e),
    list<prompt_image_info> get_all_prompt_image() throws (1:gen_exp e),
    bool delete_prompt_image(1:string ssid, 2:i64 id) throws (1:gen_exp e),
    company_address_info get_company_address_info() throws (1:gen_exp e),
    bool set_company_address_info(1:string ssid, 2:company_address_info address_info) throws (1:gen_exp e),
    register_config_info get_register_info() throws (1:gen_exp e),
    bool set_register_info(1:string ssid, 2:register_config_info register_config) throws (1:gen_exp e),
    bool need_seal_no() throws (1:gen_exp e),
    bool set_need_seal_no(1:string ssid, 2:bool need_sn) throws (1:gen_exp e),
    list<device_status> get_all_device() throws (1:gen_exp e),
    void gate_control(1:string name, 2:bool is_enter, 3:bool is_close, 4:string ssid) throws (1:gen_exp e),
    string take_picture(1:string name, 2:bool is_enter, 3:string ssid) throws (1:gen_exp e),
    string get_video(1:string name, 2:bool is_enter, 3:string begin_date, 4:string end_date, 5:string ssid) throws (1:gen_exp e),
    void manual_trigger(1:string name, 2:bool is_enter, 3:string plate, 4:string ssid) throws (1:gen_exp e),
    void reset_sm(1:string name, 2:string ssid) throws(1:gen_exp e),
    void confirm_weight(1:string name, 2:string ssid) throws(1:gen_exp e),
    void led_cast(1:string name, 2:bool is_enter, 3:string content, 4:string ssid) throws (1:gen_exp e),
    void printer_print(1:string name, 2:bool is_enter, 3:string content, 4:string qr_content, 5:string ssid) throws (1:gen_exp e),
    void set_traffic_light(1:string name, 2:bool is_enter, 3:bool is_green, 4:string ssid) throws (1:gen_exp e),
    string get_core_config(1:string ssid) throws (1:gen_exp e),
    void set_core_config(1:string ssid, 2:string config_content) throws (1:gen_exp e),
}

struct user_info {
    1:string name,
    2:string phone,
    3:string password,
    4:i64 id,
    5:i64 permission,
    6:string permission_name,
    7:bool need_change_password,
    8:list<permission_target_info> target_info,
}

struct user_permission {
    1:string name,
    2:string description,
    3:i64 id,
}

struct permission_target_info{
    1:string target,
    2:bool is_read = false,
    3:i64 id = 0,
    4:string description,
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
    list<permission_target_info> get_user_permission_target(1:i64 user_id) throws (1:gen_exp e),
    bool add_user_permission_target(1:string ssid, 2:i64 user_id, 3:permission_target_info target_info) throws (1:gen_exp e),
    void del_user_permission_target(1:string ssid, 2:i64 user_id, 3:i64 target_info_id) throws (1:gen_exp e),
    list<permission_target_info> get_all_permission_item() throws (1:gen_exp e),
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

struct contract_stuff_price {
    1:string customer_name,
    2:string stuff_name,
    3:double price,
    4:i64 id,
}

service contract_management {
    bool add_contract(1:string ssid, 2:contract_info contract) throws (1:gen_exp e),
    bool del_contract(1:string ssid, 2:i64 contract_id) throws (1:gen_exp e),
    bool update_contract(1:string ssid, 2:contract_info contract) throws (1:gen_exp e),
    list<contract_info> get_all_contract(1:string ssid) throws (1:gen_exp e),
    contract_info get_contract(1:string ssid, 2:string company_name) throws (1:gen_exp e),
    list<number_change_point> get_history(1:string ssid, 2:string company_name, 3:i64 count) throws (1:gen_exp e),
    bool change_balance(1:string ssid, 2:string company_name, 3:double new_value, 4:string reason) throws (1:gen_exp e),
    bool add_single_contract_price(1:string ssid, 2:contract_stuff_price bound_price) throws (1:gen_exp e),
    bool del_single_contract_price(1:string ssid, 2:i64 id) throws (1:gen_exp e),
    list<contract_stuff_price> get_all_single_contract_price() throws (1:gen_exp e),
    list<number_change_point> export_history(1:string ssid, 2:string begin_date, 3:string end_date, 4:string company_name) throws (1:gen_exp e),
    list<number_change_point> get_single_price_history(1:string ssid, 2:i64 single_id, 3:i64 count) throws (1:gen_exp e),
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
    12:double min_limit,
    13:double max_limit,
    14:string code,
    15:bool use_for_white_list,
    16:i64 auto_call_count,
}

struct stuff_change_point {
    1:string date,
    2:double inventory,
}

struct stuff_history {
    1:stuff_info stuff,
    2:list<stuff_change_point> change_point,
}

struct stuff_source_dest{
    1:i64 id,
    2:string name,
    3:bool is_source,
    4:string code,
}

struct stuff_inv_element {
    1:i64 id,
    2:string stuff_name,
    3:double inventory,
}

struct stuff_inv_info {
    1:i64 id,
    2:string name,
    3:list<stuff_inv_element> sie,
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
    bool add_source_dest(1:string ssid, 2:string source_dest_name, 3:bool is_source, 4:string code) throws (1:gen_exp e),
    list<stuff_source_dest> get_all_source_dest(1:bool is_source) throws (1:gen_exp e),
    bool del_source_dest(1:string ssid, 2:i64 id) throws (1:gen_exp e),
    list<string> get_white_list_stuff() throws (1:gen_exp e),
    bool set_auto_call_count(1:string ssid, 2:string stuff_name, 3:i64 auto_call_count) throws (1:gen_exp e),
    bool add_stuff_inv_info(1:string ssid, 2:string name) throws (1:gen_exp e),
    void del_stuff_inv_info(1:string ssid, 2:i64 id) throws (1:gen_exp e),
    bool update_stuff_inv_info(1:string ssid, 2:stuff_inv_info sii) throws (1:gen_exp e),
    list<stuff_inv_info> get_stuff_inv_info(1:string ssid) throws (1:gen_exp e),
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
    11:bool in_black_list,
}


service vehicle_management {
    bool add_vehicle(1:string ssid, 2:vehicle_info vehicle) throws (1:gen_exp e),
    bool update_vehicle(1:string ssid, 2:vehicle_info vehicle) throws (1:gen_exp e),
    bool del_vehicle(1:string ssid, 2:i64 vehicle_id) throws (1:gen_exp e),
    list<vehicle_info> get_all_vehicle(1:string ssid) throws (1:gen_exp e),
    list<vehicle_info> get_white_vehicle() throws (1:gen_exp e),
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
    23:string source_dest_name,
    24:bool is_sale,
    25:string bl_number,
    26:double price,
    27:string seal_no,
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
    15:string checkin_time,
    16:string call_time,
    17:string err_string,
    18:string call_user_name,
    19:i64 wait_count,
    20:string enter_picture,
    21:string exit_picture,
    22:string p_picture,
    23:string m_picture,
    24:string p_gate_name,
    25:string m_gate_name,
    26:string p_scale_name,
    27:string m_scale_name,
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

struct white_record_info {
    1:i64 id,
    2:string vehicle_number,
    3:string date,
    4:string weight,
    5:string group_name,
    6:string stuff_name,
}

service vehicle_order_center {
    list<vehicle_order_detail> get_order_by_anchor(1:string ssid, 2:i64 anchor, 3:string status_name, 4:string enter_date) throws (1:gen_exp e),
    gate_relate_info get_gate_info(1:string ssid, 2:i64 order_id) throws (1:gen_exp e),
    weight_relate_info get_weight_info(1:string ssid, 2:i64 order_id) throws (1:gen_exp e),
    bool create_vehicle_order(1:string ssid, 2:list<vehicle_order_info> order, 3:bool from_api) throws (1:gen_exp e),
    bool confirm_vehicle_order(1:string ssid, 2:list<vehicle_order_info> order) throws (1:gen_exp e),
    bool cancel_vehicle_order(1:string ssid, 2:list<vehicle_order_info> order, 3:bool from_api) throws (1:gen_exp e),
    vehicle_order_detail get_order_detail(1:string ssid, 2:string order_number) throws (1:gen_exp e),
    bool confirm_order_deliver(1:string ssid, 2:string order_number, 3:bool confirmed, 4:string inv_name) throws (1:gen_exp e),
    bool update_vehicle_order(1:string ssid, 2:vehicle_order_info order) throws (1:gen_exp e),
    bool driver_check_in(1:i64 order_id, 2:bool is_cancel, 3:string driver_id, 4:string max_load) throws (1:gen_exp e),
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
    bool record_order_source_dest(1:i64 order_id, 2:string source_dest_name) throws (1:gen_exp e),
    vehicle_order_info get_driver_opt_history(1:string driver_phone) throws (1:gen_exp e),
    list<white_record_info> get_white_record_info(1:string ssid, 2:i64 begin_date, 3:i64 end_date) throws (1:gen_exp e),
    list<vehicle_order_detail> driver_get_last_30_order_number(1:string driver_phone) throws (1:gen_exp e),
    list<vehicle_order_detail> export_order_by_condition(1:string ssid, 2:string begin_date, 3:string end_date, 4:string company_name, 5:string stuff_name) throws (1:gen_exp e),
    list<string> go_through_plugin_que(1:string ssid) throws (1:gen_exp e),
    void cancel_plugin_que(1:string ssid) throws (1:gen_exp e),
    string record_white_vehicle_stuff(1:string vehicle_number, 2:string stuff_name) throws (1:gen_exp e),
    string get_white_vehicle_stuff(1:string vehicle_number) throws (1:gen_exp e),
    bool set_seal_no(1:string ssid, 2:string order_number, 3:string seal_no) throws (1:gen_exp e),
    bool manual_push_nc(1:string order_number, 2:bool is_p) throws (1:gen_exp e),
}

struct video_param{
    1:string name,
    2:string path,
    3:i64 id,
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
    list<video_param> get_all_video_path() throws (1:gen_exp e),
    bool add_video_path(1:string ssid, 2:video_param _video_param) throws (1:gen_exp e),
    void del_video_path(1:string ssid, 2:i64 id) throws (1:gen_exp e),
    bool set_video_path(1:string ssid, 2:string video_path) throws (1:gen_exp e),
    void stop_video(1:string ssid) throws (1:gen_exp e),
}

service plugin_management {
    string run_plugin_cmd(1:string ssid, 2:string plugin_name, 3:string cmd) throws (1:gen_exp e),
    bool install_plugin(1:string ssid, 2:string plugin_name, 3:string file_name) throws (1:gen_exp e),
    void uninstall_plugin(1:string ssid, 2:string plugin_name) throws (1:gen_exp e),
    list<string> get_installed_plugins(1:string ssid) throws (1:gen_exp e),
    list<string> get_que_by_name(1:string ssid, 2:string plugin_name) throws (1:gen_exp e),
    void pop_event_from_que(1:string ssid, 2:string plugin_name) throws (1:gen_exp e),
}