#include "../abs_device_driver.h"
#include "DP_Print_inc.h"

static int g_ser_fd = -1;

void UART_SendByte(unsigned char Send_Dat)
{
    bool write_ret = false;
    if (-1 != g_ser_fd)
    {
        if (1 == send(g_ser_fd, &Send_Dat, 1, 0))
        {
            write_ret = true;
        }
    }
    if (!write_ret)
    {
        close(g_ser_fd);
        g_ser_fd = -1;
    }
}
unsigned char UART_RecByte(void)
{
    unsigned char ret = 0;
    if (-1 != g_ser_fd)
    {
        recv(g_ser_fd, &ret, 1, 0);
    }
    return ret;
}
class ser_printer : public abs_device_driver, public epoll_node_t
{
public:
    using abs_device_driver::abs_device_driver;

    virtual void printer_print(const std::string &_content, const std::string &_qr_code)
    {

        g_ser_fd = connect_to_device_tcp_server(get_device_arg("ip"), (unsigned short)atoi(get_device_arg("port").c_str()));
        InitializePrint();
        SelChineseChar();
        SetCharacterSize(1, 1);
        Set_LeftSpaceNum(12, 0);
        Sel_Align_Way(0);
        if (_content.length() > 0)
        {
            auto company_name = getenv("OEM_NAME");
            if (company_name)
            {
                Print_ASCII((utf2gbk(company_name) + "\n").c_str());
            }
            Print_ASCII(utf2gbk(_content + "\n").c_str());
        }
        if (_qr_code.length() > 0)
        {
            print_And_Line();
            Set_QRcodeMode(8);
            Set_QRCodeAdjuLevel(0x49);                                                /* 设置二维码的纠错水平 */
            Sel_Align_Way(0x01);                                                      /* 居中对齐 */
            Set_QRCodeBuffer(_qr_code.length(), (unsigned char *)(_qr_code.c_str())); /* 传输数据至编码缓存 */
            PrintQRCode();
        }
        print_And_Line();
        SelChineseChar();
        Sel_Align_Way(0);
        Print_ASCII(utf2gbk("Power By 卓创智汇过磅系统").c_str());
        print_And_Line();
        UART_SendByte(0x1D);
        UART_SendByte(0x56);
        UART_SendByte(66);
        UART_SendByte(15);
        close(g_ser_fd);
        g_ser_fd = -1;
    }
    virtual bool proc_in()
    {
        proc_msg();
        pub_device_status(true);
        return true;
    }
    virtual int get_fd() const
    {
        return abs_device_driver::get_fd();
    }
};

class printer_remote_conn : public epoll_node_t
{
public:
    virtual bool proc_in()
    {
        char buff[1];
        if (sizeof(buff) != recv(g_ser_fd, buff, sizeof(buff), 0))
        {
            perror("connection failed");
            return false;
        }
        else
        {
            return true;
        }
    }
    virtual int get_fd() const
    {
        return g_ser_fd;
    }
};

int main(int argc, char const *argv[])
{
    ser_printer sp(argc, argv, {"ip", "port"});
    epoll_sch_center sch;
    sch.add_node(sp);
    sch.run();
    sp.pub_device_status(false);
    sleep(15);
    return 0;
}
