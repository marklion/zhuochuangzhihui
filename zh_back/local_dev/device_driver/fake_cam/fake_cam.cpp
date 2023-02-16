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
        pub_vehicle_come("京A11223");
    }
};

int main(int argc, char const *argv[])
{
    fake_cam_driver fcd(argc, argv, {"ip"});
    while (1)
    {
        fcd.proc_msg();
    }

    return 0;
}
