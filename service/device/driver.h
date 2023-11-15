#if !defined(_DRIVER_H_)
#define _DRIVER_H_
#include "../lib/rpc_include.h"

class common_driver : public device_managementIf
{
protected:
    tdf_log m_log;

public:
    virtual void init_all_set() {}
  virtual void push_scale_read(const int64_t scale_id, const double weight) {}
  virtual void push_id_read(const int64_t id_id, const std::string& id_number) {}
  virtual void push_qr_read(const int64_t qr_id, const std::string& qr_content) {}
  virtual void push_plate_read(const int64_t plate_cam_id, const std::string& plate_no) {}
    common_driver(const std::string &_name) : m_log(_name, "/tmp/" + _name + ".log", "/tmp/" + _name + ".log")
    {
    }
    void log_driver(const int64_t device_id, const char *_func_name, const char *tmlt, ...)
    {
        va_list vl;
        va_start(vl, tmlt);
        char tmpbuff[8018] = {0};
        vsnprintf(tmpbuff, sizeof(tmpbuff), tmlt, vl);
        va_end(vl);
        m_log.log("pid %d call %s for device: %d,%s", getpid(), _func_name, device_id, tmpbuff);
    }
    virtual bool device_ctrl(const int64_t device_id, const bool start)
    {
        return false;
    }
    virtual bool device_is_started(const int64_t device_id)
    {
        return false;
    }
    virtual void gate_ctrl(const int64_t gate_id, const bool is_open)
    {
    }
    virtual void led_display(const int64_t led_id, const std::vector<std::string> &content)
    {
    }
    virtual void speaker_cast(const int64_t speaker_id, const std::string &content)
    {
    }
    virtual double last_scale_read(const int64_t scale_id)
    {
        return 0.001;
    }
    virtual void last_id_read(std::string &_return, const int64_t id_reader_id)
    {
    }
    virtual void last_qr_read(std::string &_return, const int64_t qr_reader_id)
    {
    }
    virtual void last_plate_read(std::string &_return, const int64_t plate_cam_id)
    {
    }
    virtual void cap_picture_slow(std::string &_return, const int64_t cam_id)
    {
    }
    virtual void video_record_slow(std::string &_return, const int64_t cam_id, const std::string &begin_date, const std::string &end_date)
    {
    }
    static void start_driver(unsigned short _port, device_managementIf *_driver)
    {
        std::shared_ptr<TMultiplexedProcessor> multi_processor(new TMultiplexedProcessor());
        multi_processor->registerProcessor("device_management", std::shared_ptr<TProcessor>(new device_managementProcessor(std::shared_ptr<device_managementIf>(_driver))));

        ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(_port));
        ::std::shared_ptr<TTransportFactory> transportFactory(new THttpServerTransportFactory());
        ::std::shared_ptr<TProtocolFactory> protocolFactory(new TJSONProtocolFactory());
        std::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(16);
        std::shared_ptr<ThreadFactory> threadFactory(new ThreadFactory());
        threadManager->threadFactory(threadFactory);
        threadManager->start();
        TThreadPoolServer tp_server(multi_processor, serverTransport, transportFactory, protocolFactory, threadManager);
        tp_server.serve();
    }
};

#endif // _DRIVER_H_
