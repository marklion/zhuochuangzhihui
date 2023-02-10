#include "../abs_device_driver.h"

class fake_cam_driver : public abs_device_driver
{
public:
    using abs_device_driver::abs_device_driver;
    std::string take_picture()
    {
        return "picture was take";
    }
    void manual_trigger()
    {
    }
};

int main(int argc, char const *argv[])
{
    fake_cam_driver device(argc, argv);
    epoll_event driver_event = {
        .events = EPOLLIN,
        .data = {.fd = device.get_fd()}};
    auto timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    timespec tv = {
        .tv_sec = 6,
        .tv_nsec = 0};
    itimerspec itv = {
        .it_interval = tv,
        .it_value = tv};
    timerfd_settime(timerfd, 0, &itv, nullptr);
    epoll_event timer_event = {
        .events = EPOLLIN,
        .data = {.fd = timerfd},
    };
    auto epfd = epoll_create(1024);
    if (epfd >= 0)
    {
        epoll_ctl(epfd, EPOLL_CTL_ADD, device.get_fd(), &driver_event);
        epoll_ctl(epfd, EPOLL_CTL_ADD, timerfd, &timer_event);

        PRINT_LOG("event loop start: %d", epfd);
        while (1)
        {
            epoll_event tmp_ev;
            if (0 < epoll_wait(epfd, &tmp_ev, 1, -1))
            {
                if (tmp_ev.data.fd == device.get_fd())
                {
                    device.proc_msg();
                }
                else if (tmp_ev.data.fd == timerfd)
                {
                    long times = 0;
                    read(timerfd, &times, sizeof(times));
                    device.pub_vehicle_come("fake vehicle come");
                }
            }
        }
    }
    return 0;
}
