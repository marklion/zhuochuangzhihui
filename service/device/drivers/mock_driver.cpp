#include "../driver.h"

class mock_driver : public common_driver
{
    std::string m_dev_name;
    void record_status(const std::string &_status)
    {
        std::string s_path = "/tmp/" + m_dev_name  + ".txt";
        int fd = open(s_path.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IROTH);
        if (fd)
        {
            write(fd, _status.data(), _status.length());
            write(fd, "\n", 1);
            close(fd);
        }
    }
public:
    tdf_log m_pub_log;
    long m_dev_id;
    bool is_gate_open = false;
    double scale_weight = 0;
    std::string id_number;
    std::string qr_content;
    std::string plate_no;
    double last_sw = 0;
    mock_driver(const std::string &_log_tag, const std::string &_dev_name, long _dev_id) : common_driver(_log_tag, _dev_id), m_pub_log(_dev_name, "/tmp/pub_log.log", "/tmp/pub_log.log"), m_dev_id(_dev_id), m_dev_name(_dev_name)
    {
        timer_wheel_add_node(
            3,
            [&](void *)
            {
                if (scale_weight != 0 || last_sw != scale_weight)
                {
                    THR_CALL_DM_BEGIN();
                    client->push_scale_read(m_dev_id, scale_weight);
                    THR_CALL_DM_END();
                    last_sw = scale_weight;
                }
            });
    }
    virtual void push_plate_read(const int64_t plate_cam_id, const std::string &plate_no)
    {
        this->plate_no = plate_no;
        plate_cam_cap(m_dev_id);
        this->plate_no.clear();
    }
    virtual void push_scale_read(const int64_t scale_id, const double weight)
    {
        scale_weight = weight;
    }
    virtual void gate_ctrl(const int64_t gate_id, const bool is_open)
    {
        log_driver(__FUNCTION__, "is_open:%d", is_open);
        m_pub_log.log(is_open ? "开门" : "关门");
        is_gate_open = !is_open;
        timer_wheel_add_node(
            2,
            [this](void *)
            {
                is_gate_open = !is_gate_open;
                this->record_status(is_gate_open ? "开门" : "关门");
            },
            true);
    }
    virtual void led_display(const int64_t led_id, const std::vector<std::string> &content)
    {
        log_driver(__FUNCTION__, "content:%s", util_join_string(content, "\n").c_str());
        record_status(util_join_string(content, "\n").c_str());
        m_pub_log.log("显示内容：'%s'", util_join_string(content, "\n").c_str());
    }
    virtual void speaker_cast(const int64_t speaker_id, const std::string &content)
    {
        record_status(content);
        m_pub_log.log("播报内容：'%s'", content.c_str());
    }
    virtual double last_scale_read(const int64_t scale_id)
    {
        log_driver(__FUNCTION__, "ret:%f", scale_weight);
        return scale_weight;
    }
    virtual void last_id_read(std::string &_return, const int64_t id_reader_id)
    {
        _return = id_number;
    }
    virtual void last_qr_read(std::string &_return, const int64_t qr_reader_id)
    {
        _return = qr_content;
    }
    virtual void last_plate_read(std::string &_return, const int64_t plate_cam_id)
    {
        _return = plate_no;
    }
    virtual void cap_picture_slow(std::string &_return, const int64_t cam_id)
    {
        _return = file_store_content("这是一个车牌照片", "txt");
        m_pub_log.log("拍照");
    }
    virtual void video_record_slow(std::string &_return, const int64_t cam_id, const std::string &begin_date, const std::string &end_date)
    {
        _return = file_store_content("这是一个从" + begin_date + "到" + end_date + "的录像", "txt");
        m_pub_log.log("录像，从%s 到%s", begin_date.c_str(), end_date.c_str());
    }
    virtual void printer_print(const int64_t printer_id, const std::string &content)
    {
        m_pub_log.log("打印内容：'%s'", content.c_str());
    }

    virtual bool gate_is_close(const int64_t gate_id)
    {
        m_pub_log.log("判断是否关门：'%s'", (is_gate_open ? "否" : "是"));
        return !is_gate_open;
    }
    virtual void plate_cam_cap(const int64_t plate_cam_id)
    {
        m_pub_log.log("手动触发抓拍，拍到车牌：'%s'", plate_no.c_str());
        THR_CALL_DM_BEGIN();
        client->push_plate_read(m_dev_id, plate_no);
        THR_CALL_DM_END();
    }
    virtual void push_id_read(const int64_t id_id, const std::string &id_number) {
        m_pub_log.log("身份证触发：%s", id_number.c_str());
        THR_CALL_DM_BEGIN();
        this->id_number = id_number;
        client->push_id_read(m_dev_id, id_number);
        THR_CALL_DM_END();
    }
};

int main(int argc, char **argv)
{
    using namespace clipp;
    unsigned short run_port = 0;
    long self_id;
    std::string set_name;
    timer_wheel_init();
    std::thread([]()
                {
            while (1)
            {
                timer_wheel_schc();
            } })
        .detach();
    auto cli = (required("-p") & value("port", run_port),
                required("-i") & value("self_id", self_id),
                required("-t") & value("abc", set_name));
    if (!parse(argc, argv, cli))
    {
        std::cerr << "Usage:\n"
                  << usage_lines(cli, argv[0])
                  << '\n';
        return -1;
    }
    auto dev = sqlite_orm::search_record<sql_device_meta>(self_id);
    if (dev)
    {
        common_driver::start_driver(run_port, new mock_driver("mock_driver" + std::to_string(self_id), dev->name, dev->get_pri_id()));
    }
    timer_wheel_fini();

    return 0;
}
