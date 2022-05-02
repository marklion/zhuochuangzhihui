#if !defined(_ZH_ID_READER_H_)
#define _ZH_ID_READER_H_

extern "C"
{
#include "libtermb.h"
}
#include "../../zh_tdf/tdf_include.h"
#include <functional>

std::string zh_read_id_no(const std::string &ip, unsigned short port);
class zh_read_id_api{
public:
    pthread_t id_thread = 0;
    bool need_exit = false;
    std::string m_ip;
    unsigned short m_port = 0;
    std::function<void(const std::string &_id)> m_func;
    zh_read_id_api(const std::string &_ip, unsigned short _port, std::function<void(const std::string &_id)> _func);
    virtual ~zh_read_id_api();
};

#endif // _ZH_ID_READER_H_
