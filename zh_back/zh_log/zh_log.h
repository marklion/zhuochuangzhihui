#if !defined(_ZH_LOG_H_)
#define _ZH_LOG_H_

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

class zh_log
{
    int m_log_stdout = 1;
    int m_log_stderr = 2;
    std::string m_module;
    void output_2_fd(const std::string &_msg, int _fd)
    {
        std::string output;
        char time_buffer[48];

        time_t now;
        time(&now);
        strftime(time_buffer, 48, "%Y/%m/%d %H:%M:%S", localtime(&now));

        output.append(time_buffer);

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
    zh_log();
    zh_log(const std::string _module, const std::string &_out_file_name, const std::string &_err_file_name)
    {
        m_log_stdout = open(_out_file_name.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
        m_log_stderr = open(_err_file_name.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
        if (m_log_stderr * m_log_stdout < 0)
        {
            std::cout << "open log file error" << std::endl;
        }
        m_module = _module;
    }
    zh_log(const std::string &_module)
    {
        m_module = _module;
    }
    ~zh_log()
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
    void log(const std::string &_log)
    {
        output_2_fd(_log, m_log_stdout);
    }
    void log(const char *_log, ...)
    {
        va_list vl;
        va_start(vl, _log);
        char tmpbuff[2048];
        vsnprintf(tmpbuff, sizeof(tmpbuff), _log, vl);
        va_end(vl);
        output_2_fd(tmpbuff, m_log_stdout);
    }
    void log_package(const char *_data, int _len) {
        char tmp[4] = {0};
        std::string out_log;
        for (int i = 0; i < _len; i++)
        {
            sprintf(tmp, "%02X ", (unsigned char)(_data[i]));
            out_log.append(tmp);
        }
        output_2_fd(out_log, m_log_stdout);
    }
    void err(const std::string &_log)
    {
        output_2_fd(_log, m_log_stderr);
    }
    void err(const char *_log, ...)
    {
        va_list vl;
        va_start(vl, _log);
        char tmpbuff[2048];
        vsnprintf(tmpbuff, sizeof(tmpbuff), _log, vl);
        va_end(vl);
        output_2_fd(tmpbuff, m_log_stderr);
    }
};

#endif // _ZH_LOG_H_
