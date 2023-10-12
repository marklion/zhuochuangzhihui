#if !defined(_TDF_INCLUDE_H_)
#define _TDF_INCLUDE_H_
#include <string>
#include <stdarg.h>
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
#include <iostream>
#include <algorithm>
#include <functional>
#include <memory>
#include <thread>
#include <mqueue.h>
#include <set>
#include <vector>

static int is_leap_year(time_t year)
{
    if (year % 4)
        return 0; /* A year not divisible by 4 is not leap. */
    else if (year % 100)
        return 1; /* If div by 4 and not 100 is surely leap. */
    else if (year % 400)
        return 0; /* If div by 100 *and* 400 is not leap. */
    else
        return 1; /* If div by 100 and not by 400 is leap. */
}

static void nolocks_localtime(struct tm *tmp, time_t t)
{
    const time_t secs_min = 60;
    const time_t secs_hour = 3600;
    const time_t secs_day = 3600 * 24;

    t -= timezone;                       /* Adjust for timezone. */
    time_t days = t / secs_day;    /* Days passed since epoch. */
    time_t seconds = t % secs_day; /* Remaining seconds. */

    tmp->tm_isdst = 0;
    tmp->tm_hour = seconds / secs_hour;
    tmp->tm_min = (seconds % secs_hour) / secs_min;
    tmp->tm_sec = (seconds % secs_hour) % secs_min;

    /* 1/1/1970 was a Thursday, that is, day 4 from the POV of the tm structure
     * where sunday = 0, so to calculate the day of the week we have to add 4
     * and take the modulo by 7. */
    tmp->tm_wday = (days + 4) % 7;

    /* Calculate the current year. */
    tmp->tm_year = 1970;
    while (1)
    {
        /* Leap years have one day more. */
        time_t days_this_year = 365 + is_leap_year(tmp->tm_year);
        if (days_this_year > days)
            break;
        days -= days_this_year;
        tmp->tm_year++;
    }
    tmp->tm_yday = days; /* Number of day of the current year. */
    /* We need to calculate in which month and day of the month we are. To do
     * so we need to skip days according to how many days there are in each
     * month, and adjust for the leap year that has one more day in February. */
    int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    mdays[1] += is_leap_year(tmp->tm_year);

    tmp->tm_mon = 0;
    while (days >= mdays[tmp->tm_mon])
    {
        days -= mdays[tmp->tm_mon];
        tmp->tm_mon++;
    }

    tmp->tm_mday = days + 1; /* Add 1 since our 'days' is zero-based. */
    tmp->tm_year -= 1900;    /* Surprisingly tm_year is year-1900. */
}

std::string get_string_from_format(const char *format, va_list vl);
class tdf_log
{
    int m_log_stdout = 1;
    int m_log_stderr = 2;
    std::string m_module;
    void output_2_fd(const std::string &_msg, int _fd) const
    {
        std::string output;
        char time_buffer[64] = {0};

        timeval usec_time;
        gettimeofday(&usec_time, nullptr);
        auto msec_time = usec_time.tv_usec / 1000;

        tm tmp_tm;
        nolocks_localtime(&tmp_tm, usec_time.tv_sec);
        strftime(time_buffer, 48, "%Y/%m/%d %H:%M:%S", &tmp_tm);
        char m_sec_string[4] = {0};
        snprintf(m_sec_string, sizeof(m_sec_string), "%03ld", msec_time);

        output.append(time_buffer);
        output.append(":" + std::string(m_sec_string));

        if (m_module.length() != 0)
        {
            output.append(std::string(" [") + m_module + "]");
        }

        if (m_log_stderr == _fd)
        {
            output.append(" [ERR] ");
        }
        else
        {
            output.append(" [INFO] ");
        }
        std::string prefix = output;
        output = "";

        std::string content = _msg;
        auto n_pos = content.find('\n');
        while (n_pos != std::string::npos)
        {
            output.append(prefix + content.substr(0, n_pos) + "\n");
            content.erase(0, n_pos + 1);
            n_pos = content.find('\n');
        }
        if (content.length() > 0)
        {
            output.append(prefix + content + "\n");
        }

        (void)write(_fd, output.c_str(), output.length());
    }

public:
    tdf_log() {}
    tdf_log(const std::string _module, const std::string &_out_file_name, const std::string &_err_file_name)
    {
        m_log_stdout = open(_out_file_name.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
        m_log_stderr = open(_err_file_name.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
        if (m_log_stderr * m_log_stdout < 0)
        {
            std::cout << "open log file error" << std::endl;
        }
        m_module = _module;
    }
    tdf_log(const std::string &_module)
    {
        m_module = _module;
    }
    ~tdf_log()
    {
        if (m_log_stdout != 1)
        {
            close(m_log_stdout);
        }
        if (m_log_stderr != 2)
        {
            close(m_log_stderr);
        }
    }
    void log(const std::string &_log) const
    {
        output_2_fd(_log, m_log_stdout);
    }
    void log(const char *_log, ...) const
    {
        va_list vl;
        va_start(vl, _log);
        auto tmpbuff = get_string_from_format(_log, vl);
        va_end(vl);
        output_2_fd(tmpbuff, m_log_stdout);
    }
    void log_package(const char *_data, int _len) const
    {
        char tmp[4] = {0};
        std::string out_log;
        for (int i = 0; i < _len; i++)
        {
            sprintf(tmp, "%02X ", (unsigned char)(_data[i]));
            out_log.append(tmp);
        }
        output_2_fd(out_log, m_log_stdout);
    }
    void err(const std::string &_log) const
    {
        output_2_fd(_log, m_log_stderr);
    }
    void err(const char *_log, ...) const
    {
        va_list vl;
        va_start(vl, _log);
        auto tmpbuff = get_string_from_format(_log, vl);
        va_end(vl);
        output_2_fd(tmpbuff, m_log_stderr);
    }
};

class Itdf_epoll_channel
{
    bool need_remove = false;

public:
    virtual bool proc_in() = 0;
    virtual void proc_err() = 0;
    virtual bool proc_out() = 0;
    void set_remove_flag()
    {
        need_remove = true;
    }
    bool get_remove_flag()
    {
        return need_remove;
    }
};

typedef std::function<void(const std::string &)> tdf_after_con_hook;
typedef std::function<void(const std::string &)> tdf_before_hup_hook;
typedef std::function<void(const std::string &, const std::string &)> tdf_data_proc;

typedef void (*tdf_timer_proc)(void *_private);

typedef void (*tdf_async_proc)(void *_private, const std::string &_chrct);

class tdf_main
{
    static tdf_main m_inst;
    tdf_main();
    pthread_t self_tid = 0;

public:
    bool open_listen(unsigned short _port, tdf_after_con_hook _con_hook, tdf_before_hup_hook _hup_hook, tdf_data_proc _data_proc);
    bool connect_remote(const std::string &_ip, unsigned short _port, tdf_after_con_hook _con_hook, tdf_before_hup_hook _hup_hook, tdf_data_proc _data_proc);
    void close_listen(unsigned short _port);
    bool run();
    void send_data(const std::string &_conn_chrct, const std::string &_data);
    void close_data(const std::string &_charct);
    void stop();
    int start_timer(int _sec, tdf_timer_proc _proc, void *_private, bool _one_time = false);
    void stop_timer(int _timer_handle);
    static tdf_main &get_inst();
    void Async_to_workthread(tdf_async_proc _func, void *_private, const std::string &_chrct);
    void Async_to_mainthread(tdf_async_proc _func, void *_private, const std::string &_chrct);
    void close_mq();
    ~tdf_main();
};

class tdf_state_machine_state;
class tdf_state_machine_lock;
class tdf_state_machine
{
    pthread_mutex_t sm_lock;
    virtual tdf_log &get_log() = 0;

public:
    static std::thread *sm_thread;
    static int sm_mq_fd;
    static std::set<tdf_state_machine *> valid_sm;
    static pthread_mutex_t valid_sm_lock;
    friend class tdf_state_machine_lock;
    void internal_trigger_sm();
    tdf_state_machine()
    {
        if (sm_mq_fd < 0)
        {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&valid_sm_lock, &attr);
            mq_attr tmp_mq_attr = {
                .mq_flags = 0,
                .mq_maxmsg = 100,
                .mq_msgsize = sizeof(void *),
                .mq_curmsgs = 0};
            int fd = mq_open("/sm_mq", O_RDWR);
            if (fd >= 0)
            {
                mq_setattr(fd, &tmp_mq_attr, nullptr);
                sm_mq_fd = fd;
            }
            else
            {
                tdf_log tmp_log("tdf_state_machine");
                tmp_log.err("failed to open mq:%s", strerror(errno));
                fd = mq_open("/sm_mq", O_RDWR | O_CREAT, 0666, &tmp_mq_attr);
                if (fd >= 0)
                {
                    sm_mq_fd = fd;
                }
                else
                {
                    tmp_log.err("failed to create mq:%s", strerror(errno));
                }
            }
        }
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&sm_lock, &attr);
        pthread_mutex_lock(&valid_sm_lock);
        valid_sm.insert(this);
        pthread_mutex_unlock(&valid_sm_lock);
    }
    std::unique_ptr<tdf_state_machine_state> m_cur_state;
    void trigger_sm();
    virtual ~tdf_state_machine()
    {
        pthread_mutex_destroy(&sm_lock);
        pthread_mutex_lock(&valid_sm_lock);
        valid_sm.erase(this);
        pthread_mutex_unlock(&valid_sm_lock);
    }
};

class tdf_state_machine_lock
{
    tdf_state_machine &related_sm;

public:
    tdf_state_machine_lock(tdf_state_machine &_sm) : related_sm(_sm)
    {
        pthread_mutex_lock(&_sm.sm_lock);
    }
    ~tdf_state_machine_lock()
    {
        pthread_mutex_unlock(&related_sm.sm_lock);
    }
};

class tdf_state_machine_state
{
public:
    virtual std::unique_ptr<tdf_state_machine_state> change_state(tdf_state_machine &_sm) = 0;
    virtual void do_action(tdf_state_machine &_sm) = 0;
    virtual void after_enter(tdf_state_machine &_sm) = 0;
    virtual void before_leave(tdf_state_machine &_sm) = 0;
    virtual std::string state_name() = 0;
};

int connect_timeout(int sockfd, struct sockaddr *serv_addr, int addrlen, int timeout);

#endif // _TDF_INCLUDE_H_
