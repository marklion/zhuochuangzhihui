#if !defined(_LOCAL_UTILS_H_)
#define _LOCAL_UTILS_H_

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <memory>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <iconv.h>
#include "../../zh_tdf/tdf_include.h"
#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../zh_vcom/zh_vcom_link.h"

#define LOCAL_DEV_MSG_TAKE_PICTURE "take_picture"
#define LOCAL_DEV_MSG_MANUAL_TRIIGER "manual_trigger"
#define LOCAL_DEV_MSG_LED_SHOW "led_show"
#define LOCAL_DEV_MSG_LED_SHOW_KEY1 "led_show_key1"
#define LOCAL_DEV_MSG_LED_SHOW_KEY2 "led_show_key2"
#define LOCAL_DEV_MSG_LED_SHOW_KEY3 "led_show_key3"
#define LOCAL_DEV_MSG_LED_SHOW_KEY4 "led_show_key4"
#define LOCAL_DEV_MSG_SPEAKER_SPEAK "speaker_speak"
#define LOCAL_DEV_MSG_SPEAKER_SPEAK_KEY "speaker_speak_key"
#define LOCAL_DEV_MSG_TRAFFIC_LIGHT_SET "traffic_light_set"
#define LOCAL_DEV_MSG_TRAFFIC_LIGHT_SET_KEY "traffic_light_set_key"
#define LOCAL_DEV_MSG_GET_VIDEO_RECORD "get_video_record"
#define LOCAL_DEV_MSG_GET_VIDEO_RECORD_KEY1 "get_video_record_key1"
#define LOCAL_DEV_MSG_GET_VIDEO_RECORD_KEY2 "get_video_record_key2"
#define LOCAL_DEV_MSG_GATE_CONTROL "gate_control"
#define LOCAL_DEV_MSG_GATE_CONTROL_KEY "gate_control_key"
#define LOCAL_DEV_MSG_GATE_IS_CLOSE "gate_is_close"
#define LOCAL_DEV_MSG_PRINTER_PRINT "printer_print"
#define LOCAL_DEV_MSG_PRINTER_PRINT_KEY1 "printer_print_key1"
#define LOCAL_DEV_MSG_PRINTER_PRINT_KEY2 "printer_print_key2"
#define LOCAL_DEV_MSG_SCALE_CUR_WEIGHT "scale_cur_weight"
#define LOCAL_DEV_EVENT_VEHICLE_COME "vehicle_come"
#define LOCAL_DEV_EVENT_VEHICLE_COME_KEY "vehicle_come_key"
#define LOCAL_DEV_EVENT_ID_COME "id_come"
#define LOCAL_DEV_EVENT_ID_COME_KEY "id_come_key"
#define LOCAL_DEV_EVENT_QR_SCAN "qr_scan"
#define LOCAL_DEV_EVENT_QR_SCAN_KEY "qr_scan_key"
#define LOCAL_DEV_EVENT_TAKE_PICTURE "take_picture_resp"
#define LOCAL_DEV_EVENT_TAKE_PICTURE_KEY "take_picture_resp_key"
#define LOCAL_DEV_EVENT_GET_VIDEO_RECORD "get_video_record_resp"
#define LOCAL_DEV_EVENT_GET_VIDEO_RECORD_KEY "get_video_record_resp_key"
#define LOCAL_DEV_EVENT_GATE_IS_CLOSE "gate_is_close_resp"
#define LOCAL_DEV_EVENT_GATE_IS_CLOSE_KEY "gate_is_close_resp_key"
#define LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT "scale_cur_weight_resp"
#define LOCAL_DEV_EVENT_SCALE_CUR_WEIGHT_KEY "scale_cur_weight_resp_key"
#define LOCAL_DEV_EVENT_DEVICE_STATUS "device_status"
#define LOCAL_DEV_EVENT_DEVICE_STATUS_KEY "device_status_key"

#define PRINT_LOG(format, ...) do {printf(format"\n", ##__VA_ARGS__);fflush(stdout);} while(0)
#define PRINT_ERR(format, ...) do {fprintf(stderr, format"\n", ##__VA_ARGS__);fflush(stderr);} while(0)


class epoll_node_t{
public:
    virtual bool proc_in() = 0;
    virtual int get_fd() const = 0;
};

class epoll_sch_center{
    int epfd = -1;
    tdf_log m_epoll_log;
public:
    epoll_sch_center():m_epoll_log("epoll_log") {
        epfd = epoll_create(1024);
    }
    virtual ~epoll_sch_center() {
        close(epfd);
    }
    bool add_node(const epoll_node_t &_node)
    {
        bool ret = false;
        epoll_event tmp_event = {
            .events = EPOLLIN,
            .data = {.ptr = (void *)&_node}
        };
        if (0 == epoll_ctl(epfd, EPOLL_CTL_ADD, _node.get_fd(), &tmp_event))
        {
            m_epoll_log.log("add %d to epoll", _node.get_fd());
            ret = true;
        }
        else
        {
            m_epoll_log.err("failed to add epoll,fd:%d", _node.get_fd());
        }

        return ret;
    }
    void del_node(const epoll_node_t &_node)
    {
        epoll_ctl(epfd, EPOLL_CTL_DEL, _node.get_fd(), nullptr);
    }
    void run() {
        while (1) {
            epoll_event tmp;
            if (0 < epoll_wait(epfd, &tmp, 1, -1))
            {
                auto p_node = (epoll_node_t *)(tmp.data.ptr);
                m_epoll_log.log("node fd:%d came event", p_node->get_fd());
                if (!p_node->proc_in())
                {
                    break;
                }
            }
        }
    }
};


std::vector<std::string> split_string(const std::string &s, const std::string &seperator);
std::string utf2gbk(const std::string &_gbk);
int connect_to_device_tcp_server(const std::string &_ip, unsigned short _port);
std::string local_dev_get_datestring(time_t _time = time(nullptr));
std::string local_dev_get_timestring(time_t _time = time(nullptr));

#endif // _LOCAL_UTILS_H_
