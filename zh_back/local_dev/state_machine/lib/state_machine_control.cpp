#include "state_machine_control.h"

static neb::CJsonObject send_and_recv(const neb::CJsonObject &_req, const std::string &_sm_name)
{
    neb::CJsonObject ret;
    std::string mq_name = "/lib_client" + std::to_string(getpid());
    auto fd = mq_open(mq_name.c_str(), O_RDWR | O_CREAT, 0666, nullptr);
    if (fd >= 0)
    {
        auto send_fd = mq_open(("/" + _sm_name).c_str(), O_WRONLY);
        if (send_fd >= 0)
        {
            auto req_msg = _req;
            req_msg.Add("device_name", mq_name.substr(1));
            mq_send(send_fd, req_msg.ToString().c_str(), req_msg.ToString().length(), 0);
            mq_close(send_fd);
            char buff[9600] = {0};
            unsigned int prio = 0;
            timespec timeout_value;
            timeval now;
            gettimeofday(&now, NULL);
            timeout_value.tv_sec = 30 + now.tv_sec;
            timeout_value.tv_nsec = 0 + now.tv_usec;
            auto recv_len = mq_timedreceive(fd, buff, sizeof(buff), &prio, &timeout_value);
            if (recv_len > 0)
            {
                ret.Parse(std::string(buff, recv_len));
            }
        }
        mq_close(fd);
        mq_unlink(mq_name.c_str());
    }

    return ret;
}

std::string sm_control_take_picture(const std::string _name, bool _is_enter)
{
    std::string ret;

    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_TAKE_PICTURE);
    req.Add(LOCAL_DEV_REQ_TAKE_PICTURE_KEY, _is_enter, _is_enter);
    auto resp = send_and_recv(req, _name);

    ret = resp(LOCAL_DEV_EVENT_TAKE_PICTURE_KEY);

    return ret;
}
std::string sm_control_get_record(const std::string _name, bool _is_enter, const std::string &_begin_date, const std::string &_end_date)
{
    std::string ret;

    neb::CJsonObject req;
    req.Add("msg_type", LOCAL_DEV_REQ_TAKE_VIDEO);
    req.Add(LOCAL_DEV_REQ_TAKE_VIDEO_KEY1, _is_enter, _is_enter);
    req.Add(LOCAL_DEV_REQ_TAKE_VIDEO_KEY2, _begin_date);
    req.Add(LOCAL_DEV_REQ_TAKE_VIDEO_KEY3, _end_date);
    auto resp = send_and_recv(req, _name);

    ret = resp(LOCAL_DEV_EVENT_GET_VIDEO_RECORD_KEY);

    return ret;
}