#if !defined(_ZH_VCOMM_LINK_H_)
#define _ZH_VCOMM_LINK_H_
#include <string>
#include <thread>

class zh_vcom_link{
    std::string pts_name;
    int ptm_fd = -1;
    int socket_fd = -1;
    std::string ip;
    unsigned short port = 0;
    std::thread *work = nullptr;
    int pipe_fd[2];
public:
    zh_vcom_link(const std::string &_ip, unsigned short _port);
    std::string get_pts();
    ~zh_vcom_link();
};

#endif // _ZH_VCOMM_LINK_H_
