#if !defined(_DEVICE_MANAGEMENT_H_)
#define _DEVICE_MANAGEMENT_H_

#include "../../base/include.h"

class device_management_handler : public device_managementIf
{
public:
    virtual bool device_ctrl(const int64_t device_id, const bool start);
    virtual bool device_is_started(const int64_t device_id);
    virtual void gate_ctrl(const int64_t gate_id, const bool is_open);
    virtual void led_display(const int64_t led_id, const std::vector<std::string> &content);
    virtual void speaker_cast(const int64_t speaker_id, const std::string &content);
    virtual double last_scale_read(const int64_t scale_id);
    virtual void last_id_read(std::string &_return, const int64_t id_reader_id);
    virtual void last_qr_read(std::string &_return, const int64_t qr_reader_id);
    virtual void last_plate_read(std::string &_return, const int64_t plate_cam_id);
    virtual void cap_picture_slow(std::string &_return, const int64_t cam_id);
    virtual void video_record_slow(std::string &_return, const int64_t cam_id, const std::string &begin_date, const std::string &end_date);
};

#endif // _DEVICE_MANAGEMENT_H_
