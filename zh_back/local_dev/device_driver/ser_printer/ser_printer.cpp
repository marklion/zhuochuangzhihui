#include "../abs_device_driver.h"
#include "DP_Print_inc.h"

static int g_ser_fd = -1;

void UART_SendByte(unsigned char Send_Dat)
{
    bool write_ret = false;
    if (-1 != g_ser_fd)
    {
        if (1 == write(g_ser_fd, &Send_Dat, 1))
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
        read(g_ser_fd, &ret, 1);
    }
    return ret;
}
class ser_printer : public abs_device_driver
{
public:
    using abs_device_driver::abs_device_driver;

    virtual void printer_print(const std::string &_content, const std::string &_qr_code)
    {
        InitializePrint();
        SelChineseChar();
        Set_ChineseCode(3);
        SetCharacterSize(0, 0);
        Set_LeftSpaceNum(128, 0);
        Sel_Align_Way(0);
        if (_content.length() > 0)
            ;
        {
            Print_ASCII(utf2gbk(_content).c_str());
        }
        Sel_Align_Way(0);
        print_And_Line();
        print_And_Line();
        Set_QRcodeMode(8);
        Set_QRCodeAdjuLevel(0x49); /* 设置二维码的纠错水平 */
        Sel_Align_Way(0x01);       /* 居中对齐 */
        if (_qr_code.length() > 0)
        {
            Set_QRCodeBuffer(_qr_code.length(), (unsigned char *)(_qr_code.c_str())); /* 传输数据至编码缓存 */
            PrintQRCode();
        }
        print_And_Line();
        Print_ASCII(utf2gbk("Power By 卓创智汇过磅系统").c_str());
        UART_SendByte(0x1D);
        UART_SendByte(0x56);
        UART_SendByte(66);
        UART_SendByte(10);
    }
};

int main(int argc, char const *argv[])
{
    ser_printer sp(argc, argv, {"ip", "port"});
    g_ser_fd = connect_to_device_tcp_server(sp.get_device_arg("ip"), (unsigned short)atoi(sp.get_device_arg("port").c_str()));
    while (g_ser_fd >= 0)
    {
        sp.proc_msg();
        sp.pub_device_status(true);
    }
    sp.pub_device_status(false);
    sleep(15);
    return 0;
}
