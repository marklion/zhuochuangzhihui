#include "tdf_include.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/time.h>
#include <utime.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <map>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <list>
#include <thread>
#include <execinfo.h>

tdf_main tdf_main::m_inst;
static int g_main2work[2];
static int g_mq_main_fd = -1;

struct tdf_async_data {
    tdf_async_proc m_proc;
    void *m_private;
    std::string m_chrct;
};

static int cur_thread_count = 0;
static void work_thread_main_loop()
{
    std::cout << "sssss" << std::endl;
    static int max_thread_count = 16;
    while (true)
    {
        tdf_async_data *pcoming = nullptr;
        read(g_main2work[0], &pcoming, sizeof(pcoming));
        while (cur_thread_count >= max_thread_count)
        {
            sleep(1);
        }
        cur_thread_count++;
        std::thread wt(
            [=]()
            {
                if (pcoming->m_proc)
                {
                    pcoming->m_proc(pcoming->m_private, pcoming->m_chrct);
                }
                delete pcoming;
                cur_thread_count--;
            });
        wt.detach();
    }
}


class tdf_data;
class tdf_listen;
struct tdf_timer_node;

static std::map<std::string, tdf_data *> g_data_map;
static std::map<unsigned short, tdf_listen *> g_listen_map;
static std::map<int, tdf_timer_node *> g_timer_map;

static int g_epoll_fd;

static bool g_exit_flag = false;
static bool g_pause_epoll = false;

tdf_main &tdf_main::get_inst()
{
    return m_inst;
}

class Itdf_io_channel
{
public:
    virtual void proc_in() = 0;
    virtual void proc_out() = 0;
    virtual ~Itdf_io_channel()
    {
    }
};

struct tdf_work_pipe_channel : public Itdf_io_channel
{
    virtual void proc_in()
    {
        tdf_async_data *pcoming = nullptr;
        mq_attr tmp;
        mq_getattr(g_mq_main_fd, &tmp);
        unsigned int priority = 0;
        auto recv_len = mq_receive(g_mq_main_fd, (char *)&pcoming, tmp.mq_msgsize, &priority);
        if (pcoming->m_proc)
        {
            pcoming->m_proc(pcoming->m_private, pcoming->m_chrct);
        }
        delete pcoming;
    }
    virtual void proc_out()
    {
    }
    std::string channel_type()
    {
        return "work_pipe";
    }
    virtual ~tdf_work_pipe_channel()
    {
    }
} g_proc_work_coming_data;

static pthread_mutex_t g_timer_lock;

void tdf_main::close_mq()
{
    epoll_ctl(g_epoll_fd, EPOLL_CTL_DEL,  g_mq_main_fd, nullptr);
}
tdf_main::tdf_main()
{
    g_epoll_fd = epoll_create(1);
    pipe(g_main2work);
    mq_attr tmp_mq_attr = {
        .mq_flags = 0,
        .mq_maxmsg = 100,
        .mq_msgsize = sizeof(void *),
        .mq_curmsgs = 0};
    int fd = mq_open("/main_mq", O_RDWR);
    if (fd >= 0)
    {
        mq_setattr(fd, &tmp_mq_attr, nullptr);
        g_mq_main_fd = fd;
    }
    else
    {
        tdf_log tmp_log("tdf_main_mq");
        tmp_log.err("failed to open mq:%s", strerror(errno));
        fd = mq_open("/main_mq", O_RDWR | O_CREAT, 0666, &tmp_mq_attr);
        if (fd >= 0)
        {
            g_mq_main_fd = fd;
        }
        else
        {
            tmp_log.err("failed to create mq:%s", strerror(errno));
        }
    }
    if (fd >= 0)
    {
        struct epoll_event ev = {
            .events = EPOLLIN,
            .data = {.ptr = &g_proc_work_coming_data}};

        epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, g_mq_main_fd, &ev);
    }
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_timer_lock, &attr);
}
struct tdf_timer_lock
{
    tdf_timer_lock()
    {
        pthread_mutex_lock(&g_timer_lock);
    }
    ~tdf_timer_lock()
    {
        pthread_mutex_unlock(&g_timer_lock);
    }
};
struct tdf_timer_node : public Itdf_io_channel
{
    tdf_timer_proc m_proc = nullptr;
    void *m_private = nullptr;
    int m_handle = -1;
    bool m_one_time = false;
    bool need_delete = false;
    std::string channel_type()
    {
        return "timer fd : " + std::to_string(m_handle);
    }
    virtual ~tdf_timer_node()
    {
    }
    void proc_in()
    {
        if (need_delete)
        {
            uint64_t times = 0;
            read(m_handle, &times, sizeof(times));
            tdf_timer_lock a;
            epoll_ctl(g_epoll_fd, EPOLL_CTL_DEL, m_handle, nullptr);
            close(m_handle);
            g_timer_map.erase(m_handle);
            tdf_log tmp("timer_start_stop");
            tmp.log("close timer fd:%d, ct_p:%p", m_handle, this);
            g_pause_epoll = true;
            delete this;
        }
        else
        {
            uint64_t times = 0;
            if (sizeof(uint64_t) == read(m_handle, &times, sizeof(times)))
            {
                int cur_handle = m_handle;
                while (times--)
                {
                    {
                        tdf_timer_lock a;
                        if (nullptr == g_timer_map[cur_handle])
                        {
                            break;
                        }
                    }
                    m_proc(m_private);
                    if (m_one_time)
                    {
                        tdf_main::get_inst().stop_timer(m_handle);
                        return;
                    }
                }
            }
        }
    }
    void proc_out()
    {
    }
};

class tdf_data : public Itdf_io_channel
{
public:
    int m_fd = -1;
    std::string m_chrct;
    tdf_data_proc m_data_proc;
    tdf_before_hup_hook m_hup_hook;
    std::string out_buff;
    std::string channel_type()
    {
        return "data tcp :" + std::to_string(m_fd);
    }
    tdf_data(int _fd, const std::string &_chrct, tdf_data_proc _data_proc, tdf_before_hup_hook _hup_hook) : m_fd(_fd), m_chrct(_chrct), m_data_proc(_data_proc), m_hup_hook(_hup_hook) {}
    ~tdf_data()
    {
        if (m_fd >= 0)
        {
            close(m_fd);
        }
    }
    void proc_in()
    {
        std::string recv_buff;
        char tmp_buff[256];
        int tmp_len = 0;

        while (0 < (tmp_len = recv(m_fd, tmp_buff, sizeof(tmp_buff), MSG_DONTWAIT)))
        {
            recv_buff.append(tmp_buff, tmp_len);
        }

        if (0 >= recv_buff.size())
        {
            tdf_main::get_inst().close_data(m_chrct);
            return;
        }
        else
        {
            if (m_data_proc)
            {
                m_data_proc(m_chrct, recv_buff);
            }
        }
    }
    void proc_out()
    {
        int orig_len = out_buff.size();
        auto send_len = send(m_fd, out_buff.data(), orig_len, MSG_DONTWAIT);
        if (send_len < 0)
        {
            tdf_main::get_inst().close_data(m_chrct);
            return;
        }
        if (send_len == orig_len)
        {
            struct epoll_event ev = {
                .events = EPOLLIN,
                .data = {.ptr = this}};
            epoll_ctl(g_epoll_fd, EPOLL_CTL_MOD, m_fd, &ev);
        }
        out_buff.erase(0, send_len);
    }
};

class tdf_listen : public Itdf_io_channel
{
public:
    unsigned short m_port;
    tdf_after_con_hook m_con_hook;
    tdf_before_hup_hook m_hup_hook;
    tdf_data_proc m_data_proc;
    int m_fd = -1;
    std::string channel_type()
    {
        return "listen fd:" + std::to_string(m_fd);
    }
    tdf_listen(unsigned short _port,
               tdf_after_con_hook _con_hook,
               tdf_before_hup_hook _hup_hook,
               tdf_data_proc _data_proc) : m_port(_port),
                                           m_con_hook(_con_hook),
                                           m_hup_hook(_hup_hook),
                                           m_data_proc(_data_proc)
    {
    }
    virtual ~tdf_listen()
    {
    }
    bool set_listen()
    {
        bool ret = false;
        int fd = socket(AF_INET, SOCK_STREAM, 0);

        if (fd >= 0)
        {
            struct sockaddr_in server_addr = {
                .sin_family = AF_INET,
                .sin_port = ntohs(m_port),
                .sin_addr = {.s_addr = INADDR_ANY}};
            int opt = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
            if (0 == bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
            {
                if (0 == listen(fd, 128))
                {
                    m_fd = fd;
                    ret = true;
                }
            }
            if (ret == false)
            {
                close(fd);
            }
        }

        return ret;
    }
    void clear_listen()
    {
        close(m_fd);
    }
    void proc_in()
    {
        struct sockaddr_in client_addr;
        unsigned int addr_len = sizeof(client_addr);
        int data_fd = accept(m_fd, (sockaddr *)&client_addr, &addr_len);
        if (0 <= data_fd)
        {
            std::string chcrt;
            chcrt.append(inet_ntoa(client_addr.sin_addr));
            chcrt.append(std::to_string(ntohs(client_addr.sin_port)));
            chcrt.append(std::to_string(data_fd));
            auto data_channel = new tdf_data(data_fd, chcrt, m_data_proc, m_hup_hook);

            struct epoll_event ev = {
                .events = EPOLLIN,
                .data = {.ptr = data_channel}};

            if (0 == epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, data_fd, &ev))
            {
                g_data_map[chcrt] = data_channel;
                if (m_con_hook != nullptr)
                {
                    m_con_hook(chcrt);
                }
            }
            else
            {
                delete data_channel;
            }
        }
    }
    void proc_out()
    {
    }
};

bool tdf_main::open_listen(unsigned short _port, tdf_after_con_hook _con_hook, tdf_before_hup_hook _hup_hook, tdf_data_proc _data_proc)
{
    bool ret = false;
    auto listen_channel = new tdf_listen(_port, _con_hook, _hup_hook, _data_proc);
    struct epoll_event ev = {
        .events = EPOLLIN,
        .data = {.ptr = listen_channel}};

    if (listen_channel->set_listen())
    {
        if (0 == epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, listen_channel->m_fd, &ev))
        {
            g_listen_map[_port] = listen_channel;
            ret = true;
        }
        else
        {
            delete listen_channel;
        }
    }
    else
    {
        delete listen_channel;
    }

    return ret;
}
void tdf_main::close_listen(unsigned short _port)
{
    auto listen_channel = g_listen_map[_port];

    if (nullptr != listen_channel)
    {
        epoll_ctl(g_epoll_fd, EPOLL_CTL_DEL, listen_channel->m_fd, nullptr);
        listen_channel->clear_listen();
        delete listen_channel;
        g_listen_map.erase(_port);
        g_pause_epoll = true;
    }
}

bool tdf_main::run()
{
    bool ret = true;
    epoll_event evs[128];
    std::thread(work_thread_main_loop).detach();
    self_tid = pthread_self();
    while (false == g_exit_flag)
    {
        int ev_num = epoll_wait(g_epoll_fd, evs, 128, -1);
        if (ev_num <= 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            else
            {
                ret = false;
                break;
            }
        }
        g_pause_epoll = false;
        for (int i = 0; i < ev_num; i++)
        {
            auto channel = (Itdf_io_channel *)(evs[i].data.ptr);
            if (evs[i].events & EPOLLIN)
            {
                tdf_log tmp_log("epoll_run");
                tmp_log.log("proc channel:%p", channel);
                channel->proc_in();
            }
            if (g_pause_epoll)
            {
                break;
            }
            if (evs[i].events & EPOLLOUT)
            {
                channel->proc_out();
            }
        }
    }

    return ret;
}

void tdf_main::send_data(const std::string &_conn_chrct, const std::string &_data)
{
    auto channel = g_data_map[_conn_chrct];
    if (channel)
    {
        auto send_len = send(channel->m_fd, _data.data(), _data.size(), MSG_DONTWAIT);
        if (0 > send_len)
        {
            close_data(_conn_chrct);
        }
        else if (send_len < _data.size())
        {
            channel->out_buff.append(_data.begin() + send_len, _data.end());
            struct epoll_event ev = {
                .events = EPOLLIN | EPOLLOUT,
                .data = {.ptr = channel}};
            epoll_ctl(g_epoll_fd, EPOLL_CTL_MOD, channel->m_fd, &ev);
        }
    }
}

void tdf_main::close_data(const std::string &_charct)
{
    std::string tmp_chrct(_charct);
    tdf_main::get_inst().Async_to_mainthread(
        [](void *_private, const std::string &_chrct)
        {
            auto channel = g_data_map[_chrct];
            if (nullptr != channel)
            {
                if (channel->m_hup_hook)
                {
                    channel->m_hup_hook(_chrct);
                }
                g_data_map.erase(_chrct);
                epoll_ctl(g_epoll_fd, EPOLL_CTL_DEL, channel->m_fd, nullptr);
                g_pause_epoll = true;
                delete channel;
            }
        },
        nullptr, tmp_chrct);
}
void tdf_main::stop()
{
    std::list<unsigned short> ports;
    std::list<std::string> chrcts;

    for (auto itr : g_listen_map)
    {
        ports.push_back(itr.first);
    }
    for (auto itr : g_data_map)
    {
        chrcts.push_back(itr.first);
    }
    for (auto itr : ports)
    {
        close_listen(itr);
    }
    for (auto itr : chrcts)
    {
        close_data(itr);
    }
    g_exit_flag = true;
}
int connect_timeout(int sockfd, struct sockaddr *serv_addr, int addrlen, int timeout)
{
    int ret = -1;
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    int n = connect(sockfd, serv_addr, sizeof(*serv_addr));
    if (n < 0)
    {
        /*
        EINPROGRESS表示connect正在尝试连接
        #define EWOULDBLOCK EAGAIN Operation would block
        */
        if (errno != EINPROGRESS && errno != EWOULDBLOCK)
        {
            ret = -1;
        }
        else
        {

            struct timeval tv;
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
            fd_set wset;
            FD_ZERO(&wset);
            FD_SET(sockfd, &wset);
            n = select(sockfd + 1, NULL, &wset, NULL, &tv);
            if (0 == n)
            {
                ret = 0;
            }
            else if (1 == n)
            {
                ret = 1;
            }
        }
    }
    else
    {
        ret = 1;
    }
    fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK); // 恢复为阻塞模式
    return ret;
}
bool tdf_main::connect_remote(const std::string &_ip, unsigned short _port, tdf_after_con_hook _con_hook, tdf_before_hup_hook _hup_hook, tdf_data_proc _data_proc)
{
    bool ret = false;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd >= 0)
    {
        struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = ntohs(_port),
            .sin_addr = {.s_addr = inet_addr(_ip.c_str())}};
        if (1 == connect_timeout(socket_fd, (sockaddr *)&server_addr, sizeof(server_addr), 700))
        {
            std::string chcrt;
            chcrt.append(inet_ntoa(server_addr.sin_addr));
            chcrt.append(" " + std::to_string(ntohs(server_addr.sin_port)));
            chcrt.append(std::to_string(socket_fd));
            auto data_channel = new tdf_data(socket_fd, chcrt, _data_proc, _hup_hook);
            struct epoll_event ev = {
                .events = EPOLLIN,
                .data = {.ptr = data_channel}};
            if (_con_hook)
            {
                _con_hook(chcrt);
            }
            struct socket_async_param
            {
                epoll_event ev;
                int socket_fd;
                std::string chrct;
            } *p_socket_ap = new socket_async_param();
            memcpy(&(p_socket_ap->ev), &ev, sizeof(ev));
            p_socket_ap->socket_fd = socket_fd;
            p_socket_ap->chrct = chcrt;
            tdf_main::get_inst().Async_to_mainthread(
                [](void *_private, const std::string &)
                {
                    auto psock_ap = (socket_async_param *)_private;
                    if (0 == epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, psock_ap->socket_fd, &(psock_ap->ev)))
                    {
                        g_data_map[psock_ap->chrct] = (tdf_data *)(psock_ap->ev.data.ptr);
                    }
                    else
                    {
                        delete (tdf_data *)(psock_ap->ev.data.ptr);
                    }
                    delete psock_ap;
                },
                p_socket_ap, "");
            ret = true;
        }
        else
        {
            close(socket_fd);
        }
    }

    return ret;
}
int tdf_main::start_timer(int _sec, tdf_timer_proc _proc, void *_private, bool _one_time)
{
    int ret = -1;
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timer_fd >= 0)
    {
        timespec tv = {
            .tv_sec = _sec,
            .tv_nsec = 0};
        itimerspec itv = {
            .it_interval = tv,
            .it_value = tv};
        ret = timerfd_settime(timer_fd, 0, &itv, nullptr);
        struct timer_async_param
        {
            int fd = 0;
            tdf_timer_proc proc = nullptr;
            void *m_private = nullptr;
            bool one_time = false;
        } *async_param = new timer_async_param();
        async_param->fd = timer_fd;
        async_param->proc = _proc;
        async_param->m_private = _private;
        async_param->one_time = _one_time;
        if (0 == ret)
        {
            tdf_main::get_inst().Async_to_mainthread(
                [](void *_private, const std::string &_chrct)
                {
                    auto pasync_param = (timer_async_param *)(_private);
                    auto pnode = new tdf_timer_node();
                    pnode->m_private = pasync_param->m_private;
                    pnode->m_proc = pasync_param->proc;
                    pnode->m_handle = pasync_param->fd;
                    pnode->m_one_time = pasync_param->one_time;

                    struct epoll_event ev = {
                        .events = EPOLLIN,
                        .data = {.ptr = pnode}};

                    if (0 == epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, pasync_param->fd, &ev))
                    {
                        tdf_log tmp("timer_start_stop");
                        tmp.log("open timer fd:%d, ct_p:%p", pasync_param->fd, pnode);
                        g_timer_map[pasync_param->fd] = pnode;
                    }
                    else
                    {
                        close(pasync_param->fd);
                        delete pnode;
                    }
                    delete pasync_param;
                },
                async_param, "");
        }
    }

    return timer_fd;
}

void tdf_main::stop_timer(int _timer_handle)
{
    struct stop_timer_param
    {
        int fd = -1;
    } *async_param = new stop_timer_param();
    async_param->fd = _timer_handle;

    tdf_main::get_inst().Async_to_mainthread(
        [](void *_private, const std::string &_chrct)
        {
            auto pasync_param = (stop_timer_param *)(_private);

            auto pnode = g_timer_map[pasync_param->fd];
            if (nullptr != pnode)
            {
                pnode->need_delete = true;
            }
            delete pasync_param;
        },
        async_param, "");
}

tdf_main::~tdf_main()
{
}
void tdf_main::Async_to_workthread(tdf_async_proc _func, void *_private, const std::string &_chrct)
{
    auto pout = new tdf_async_data();
    pout->m_private = _private;
    pout->m_proc = _func;
    pout->m_chrct = _chrct;
    write(g_main2work[1], &pout, sizeof(pout));
}
void tdf_main::Async_to_mainthread(tdf_async_proc _func, void *_private, const std::string &_chrct)
{
    if (self_tid == pthread_self())
    {
        _func(_private, _chrct);
    }
    else
    {
        auto pout = new tdf_async_data();
        pout->m_private = _private;
        pout->m_proc = _func;
        pout->m_chrct = _chrct;
        mq_send(g_mq_main_fd, (const char *)&pout, sizeof(pout), 1);
    }
}
std::string get_string_from_format(const char *format, va_list vl_orig)
{
    std::string ret;
    va_list vl;
    va_copy(vl, vl_orig);

    auto vl_len = vsnprintf(nullptr, 0, format, vl) + 1;
    va_end(vl);
    char *tmpbuff = (char *)calloc(1UL, vl_len);
    if (tmpbuff)
    {
        vsnprintf(tmpbuff, vl_len, format, vl_orig);
        ret.assign(tmpbuff);
        free(tmpbuff);
    }

    return ret;
}

pthread_mutex_t tdf_state_machine::valid_sm_lock;
std::set<tdf_state_machine *> tdf_state_machine::valid_sm;
int tdf_state_machine::sm_mq_fd = -1;

void tdf_state_machine::internal_trigger_sm()
{
    if (m_cur_state)
    {
        get_log().log("do action in %s", m_cur_state->state_name().c_str());
        m_cur_state->do_action(*this);
        auto next_state = m_cur_state->change_state(*this);
        if (next_state)
        {
            get_log().log("leave %s", m_cur_state->state_name().c_str());
            m_cur_state->before_leave(*this);
            m_cur_state.reset(next_state.release());
            get_log().log("enter %s", m_cur_state->state_name().c_str());
            m_cur_state->after_enter(*this);
            internal_trigger_sm();
        }
    }
}

void tdf_state_machine::trigger_sm()
{
    auto this_var = this;
    if (0 != mq_send(sm_mq_fd, (char *)&this_var, sizeof(this), 1))
    {
        get_log().err("failed to send msg to mq: %s", strerror(errno));
    }
}

std::thread *tdf_state_machine::sm_thread = nullptr;
void __attribute__((constructor)) tdf_lib_init(void)
{
    tzset();
    tdf_state_machine::sm_thread = new std::thread(
        []()
        {
            tdf_log tmp_log("sm_thread");
            while (true)
            {
                tdf_state_machine *sm = nullptr;
                unsigned int priority = 0;
                if (tdf_state_machine::sm_mq_fd >= 0)
                {
                    mq_attr tmp;
                    mq_getattr(tdf_state_machine::sm_mq_fd, &tmp);
                    auto recv_len = mq_receive(tdf_state_machine::sm_mq_fd, (char *)&sm, tmp.mq_msgsize, &priority);
                    if (recv_len == sizeof(sm))
                    {
                        pthread_mutex_lock(&tdf_state_machine::valid_sm_lock);
                        auto sm_found = tdf_state_machine::valid_sm.find(sm);
                        if (sm_found != tdf_state_machine::valid_sm.end())
                        {
                            tdf_state_machine_lock lock(*sm);
                            sm->internal_trigger_sm();
                        }
                        pthread_mutex_unlock(&tdf_state_machine::valid_sm_lock);
                    }
                    else
                    {
                        tmp_log.err("failed to recv msg:%s", strerror(errno));
                    }
                }
                else
                {
                    sleep(1);
                }
            }
        });
    tdf_state_machine::sm_thread->detach();
}