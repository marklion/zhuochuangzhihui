#include "../abs_device_driver.h"
class kl_scale : public abs_device_driver, public epoll_node_t
{
public:
    double g_cur_weight = 0;
    using abs_device_driver::abs_device_driver;
    virtual double scale_cur_weight()
    {
        return g_cur_weight * std::stod(get_device_arg("rate"));
    }
    virtual bool proc_in()
    {
        proc_msg();
        return true;
    }
    virtual int get_fd() const
    {
        return abs_device_driver::get_fd();
    }
} *g_p_ks = nullptr;
class scale_conn : public epoll_node_t
{
    int fd = -1;
public:
    bool need_revert = false;
    bool connect_to_scale(const std::string &_ip, unsigned short _port)
    {
        bool ret = false;
        auto tmp_fd = connect_to_device_tcp_server(_ip, _port);
        if (tmp_fd >= 0)
        {
            fd = tmp_fd;
            g_p_ks->pub_device_status(true);
            ret = true;
        }
        else
        {
            ret = false;
            g_p_ks->pub_device_status(false);
        }

        return ret;
    }
    virtual bool proc_in()
    {
        bool ret = false;
        char buff[2048] = {0};
        std::string one_frame;
        auto read_len = recv(fd, buff, sizeof(buff), 0);
        if (read_len >= 12)
        {
            auto p_tail = buff + read_len - 1;
            auto p_head = buff;
            while (*p_tail != 0x03 && p_tail != p_head)
            {
                p_tail--;
            }
            if (p_tail - p_head >= 11)
            {
                p_head = p_tail - 11;
                if (*p_head == 0x02)
                {
                    one_frame.assign(p_head, 12);
                    double weight = 0;
                    auto sign_flag = one_frame[1];
                    if (need_revert)
                    {
                        auto begin = 2;
                        auto end = 7;
                        while (end > begin)
                        {
                            auto tmp = one_frame[begin];
                            one_frame[begin] = one_frame[end];
                            one_frame[end] = tmp;
                            begin++;
                            end--;
                        }
                    }
                    for (auto i = 0; i < 6; i++)
                    {
                        auto dig = one_frame[2 + i];
                        weight += (dig - '0') * pow(10, (5 - i));
                    }
                    for (auto i = 0; i < (one_frame[8] - '0'); i++)
                    {
                        weight /= 10;
                    }

                    if (sign_flag == '-')
                    {
                        weight = 0 - weight;
                    }
                    g_p_ks->g_cur_weight = weight;
                }
            }
            ret = true;
        }
        else
        {
            g_p_ks->pub_device_status(false);
        }

        return ret;
    }
    virtual int get_fd() const
    {
        return fd;
    }
};

int main(int argc, char const *argv[])
{
    g_p_ks = new kl_scale(argc, argv, {"ip", "port", "rate", "revert"});
    scale_conn sc;
    if (g_p_ks->get_device_arg("revert") == "true")
    {
        sc.need_revert = true;
    }
    if (sc.connect_to_scale(g_p_ks->get_device_arg("ip"), (unsigned short)(atoi(g_p_ks->get_device_arg("port").c_str()))))
    {
        epoll_sch_center sch;
        sch.add_node(sc);
        sch.add_node(*g_p_ks);
        sch.run();
    }
    g_p_ks->pub_device_status(false);
    sleep(15);

    return 0;
}
