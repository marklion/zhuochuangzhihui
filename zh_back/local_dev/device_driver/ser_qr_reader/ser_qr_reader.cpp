#include "../abs_device_driver.h"

class ser_qr_reader : public abs_device_driver
{
public:
    using abs_device_driver::abs_device_driver;
};

int main(int argc, char const *argv[])
{
    ser_qr_reader sqr(argc, argv, {"ip", "port"});
    auto sock_fd = connect_to_device_tcp_server(sqr.get_device_arg("ip"), (unsigned short)atoi(sqr.get_device_arg("port").c_str()));
    while (sock_fd >= 0)
    {
        char buff[1024];
        auto recv_len = recv(sock_fd, buff, sizeof(buff), 0);
        if (recv_len > 0)
        {
            sqr.pub_device_status(true);
            sqr.pub_qr_scan(std::string(buff, recv_len));
        }
        else
        {
            perror("faild while recv:");
            close(sock_fd);
            sock_fd = -1;
        }
    }
    sqr.pub_device_status(false);
    sleep(15);

    return 0;
}
