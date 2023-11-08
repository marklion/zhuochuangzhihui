exception gen_exp {
    1:string msg,
}

struct stuff_config {
    1:i64 id,
    2:string stuff_name,
    3:double inventory,
    4:bool need_enter_weight,
    5:double price,
    6:double expect_weight,
    7:bool need_manual_scale,
    8:double min_limit,
    9:double max_limit,
    10:string code,
    11:bool use_for_white_list,
    12:bool auto_call_count,
}

struct rbac_user {
    1:string name,
    2:i64 id,
    3:string phone,
    4:list<string> role_name,
    5:string md5_password,
}

struct rbac_permission {
    1:string name,
    2:string text_name,
    3:i64 id,
    4:i64 is_module,
    5:list<string> role_name,
}

struct rbac_role{
    1:string role_name,
    2:i64 id,
    3:list<rbac_user> all_user,
    4:list<rbac_permission> author_modules,
    5:list<rbac_permission> author_resouces,
    6:bool read_only,
}


service rbac_center{
    bool has_permission(1:string token, 2:string target_module, 3:string target_resouce, 4:bool is_write) throws (1:gen_exp e),
    bool add_role(1:rbac_role new_one) throws (1:gen_exp e),
    bool del_role(1:i64 role_id) throws (1:gen_exp e),
    list<rbac_role> get_all_roles() throws (1:gen_exp e),
    bool add_user_to_role(1:i64 role_id, 2:i64 user_id) throws (1:gen_exp e),
    bool del_user_from_role(1:i64 role_id, 2:i64 user_id) throws (1:gen_exp e),
    bool add_role_permission(1:i64 role_id, 2:i64 perm_id) throws (1:gen_exp e),
    bool del_role_permission(1:i64 role_id, 2:i64 perm_id) throws (1:gen_exp e),
    bool add_user(1:rbac_user new_one) throws (1:gen_exp e),
    void del_user(1:i64 user_id) throws (1:gen_exp e),
    list<rbac_user> get_all_user() throws (1:gen_exp e),
    list<rbac_permission> get_all_permission() throws (1:gen_exp e),
    string login(1:string phone, 2:string pwd) throws (1:gen_exp e),
}

struct device_driver {
    1:string name,
    2:i64 id,
    3:string path,
}

struct device_meta{
    1:string name,
    2:i64 id,
    3:string driver_args,
    4:device_driver driver,
}

struct device_couple {
    1:device_meta front,
    2:device_meta back;
}

struct device_scale_set {
    1:string name,
    2:i64 id,
    3:device_couple plate_cam,
    4:device_couple video_cam,
    5:device_couple led,
    6:device_couple speaker,
    7:device_couple id_reader,
    8:device_couple qr_reader,
    9:device_couple gate,
    10:device_couple printer,
    11:device_meta scale,
}

struct device_gate_set{
    1:string name,
    2:i64 id,
    3:device_couple plate_cam,
    4:device_couple video_cam,
    5:device_couple led,
    6:device_couple speaker,
    7:device_couple id_reader,
    8:device_couple qr_reader,
    9:device_couple gate,
}

struct contract_config {
    1:string name,
    2:bool is_sale,
    3:string attachment,
    4:i64 id,
    5:string code,
    6:string admin_name_phone,
    8:double balance,
    9:double credit,
    10:list<stuff_config> follow_stuffs,
}

struct vehicle_config {
    1:string plate_no,
    2:i64 id,
    3:string back_plate_no,
    4:string driver_name,
    5:string driver_phone,
    6:string driver_id,
    7:bool in_black_list,
    8:bool in_white_list,
}

service config_management{
    list<stuff_config> get_stuff_config() throws (1:gen_exp e),
    bool add_stuff_config(1:stuff_config new_one) throws (1:gen_exp e),
    bool del_stuff_config(1:i64 id) throws (1:gen_exp e),
    list<device_scale_set> get_scale_config() throws (1:gen_exp e),
    list<device_gate_set> get_gate_config() throws (1:gen_exp e),
    list<device_driver> get_all_driver() throws (1:gen_exp e),
    bool add_device_set(1:string name, 2:bool is_scale) throws(1:gen_exp e),
    bool del_device_set(1:i64 set_id) throws (1:gen_exp e),
    bool add_device_to_set(1:string name, 2:string driver_args, 3:i64 driver_id, 4:i64 set_id, 5:string use_for) throws (1:gen_exp e),
    bool del_device_from_set(1:i64 device_id) throws (1:gen_exp e),
    list<contract_config> get_contract_config() throws(1:gen_exp e),
    list<vehicle_config> get_vehicle_config() throws (1:gen_exp e),
    bool add_contract(1:contract_config new_one) throws (1:gen_exp e),
    void del_contract(1:i64 contract_id) throws (1:gen_exp e),
    bool update_contract(1:contract_config input) throws(1:gen_exp e),
    bool add_vehicle(1:vehicle_config new_one) throws(1:gen_exp e),
    void del_vehicle(1:i64 vehicle_id) throws(1:gen_exp e),
    bool update_vehicle(1:vehicle_config input) throws(1:gen_exp e),
}

service order_center {

}

service device_management {
    bool device_ctrl(1:i64 device_id, 2:bool start) throws (1:gen_exp e),
    bool device_is_started(1:i64 device_id) throws (1:gen_exp e),
    oneway void gate_ctrl(1:i64 gate_id, 2:bool is_open),
    oneway void led_display(1:i64 led_id, 2:list<string> content),
    oneway void speaker_cast(1:i64 speaker_id, 2:string content),
    double last_scale_read(1:i64 scale_id) throws (1:gen_exp e),
    string last_id_read(1:i64 id_reader_id) throws (1:gen_exp e),
    string last_qr_read(1:i64 qr_reader_id) throws (1:gen_exp e),
    string last_plate_read(1:i64 plate_cam_id) throws (1:gen_exp e),
    string cap_picture_slow(1:i64 cam_id) throws (1:gen_exp e),
    string video_record_slow(1:i64 cam_id, 2:string begin_date, 3:string end_date) throws (1:gen_exp e),
}