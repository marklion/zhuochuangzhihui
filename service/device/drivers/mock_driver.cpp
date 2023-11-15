#include "../driver.h"

class mock_driver : public common_driver
{
public:
    using common_driver::common_driver;
    virtual void gate_ctrl(const int64_t gate_id, const bool is_open)
    {
        log_driver(gate_id, __FUNCTION__, "is_open:%d", is_open);
    }
    virtual void led_display(const int64_t led_id, const std::vector<std::string> &content)
    {
        log_driver(led_id, __FUNCTION__, "content:%s", util_join_string(content, "\n").c_str());
    }
    virtual void speaker_cast(const int64_t speaker_id, const std::string &content)
    {
    }
    virtual double last_scale_read(const int64_t scale_id)
    {
        log_driver(scale_id, __FUNCTION__, "ret:%f", 15);
        return 15;
    }
    virtual void last_id_read(std::string &_return, const int64_t id_reader_id)
    {
    }
    virtual void last_qr_read(std::string &_return, const int64_t qr_reader_id)
    {
    }
    virtual void last_plate_read(std::string &_return, const int64_t plate_cam_id)
    {
    }
    virtual void cap_picture_slow(std::string &_return, const int64_t cam_id)
    {
    }
    virtual void video_record_slow(std::string &_return, const int64_t cam_id, const std::string &begin_date, const std::string &end_date)
    {
    }
};

int main(int argc, char **argv)
{
    using namespace clipp;
    unsigned short run_port = 0;
    unsigned short self_id;
    auto cli = (required("-p") & value("port", run_port),
                required("-i") & value("self_id", self_id),
                required("-t") & value("abc", self_id));
    if (!parse(argc, argv, cli))
    {
        std::cerr << "Usage:\n"
                  << usage_lines(cli, argv[0])
                  << '\n';
        return -1;
    }

    common_driver::start_driver(run_port, new mock_driver("mock_driver" + std::to_string(self_id)));

    return 0;
}
