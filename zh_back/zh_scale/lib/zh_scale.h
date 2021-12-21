#if !defined(_ZH_SCALE_H_)
#define _ZH_SCALE_H_

#include <string>
#include <memory>
#include <vector>

class zh_scale_if{
public:
    virtual double get_weight(const std::string &_scale_ip, unsigned short _port) = 0;
    virtual void clean_scale(const std::string &_scale_ip, unsigned short _port) = 0;
    virtual std::unique_ptr<zh_scale_if> clone_self() = 0;

    static std::unique_ptr<zh_scale_if> get_scale(const std::string &_brand);
    static std::vector<std::string> get_all_brand();
};

double get_current_weight(const std::string &_scale_ip, unsigned short _port, const std::string &_brand);
void clean_scale_weight(const std::string &_scale_ip, unsigned short _port, const std::string &_brand);

#endif // _ZH_SCALE_H_


