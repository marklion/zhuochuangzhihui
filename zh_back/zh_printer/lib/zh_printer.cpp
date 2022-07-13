#include "zh_printer.h"
#include <unistd.h>
#include "../../zh_rpc_server/zh_rpc_util.h"
#include "../../zh_tdf/tdf_include.h"
#include "DP_Print_inc.h"
static int g_ser_fd = -1;
static std::string g_focus_printer_ip;

static pthread_mutex_t g_plock = PTHREAD_MUTEX_INITIALIZER;

void UART_SendByte(unsigned char Send_Dat)
{
    bool write_ret = false;
    if (-1 != g_ser_fd) {
        if (1 == write(g_ser_fd, &Send_Dat, 1))
        {
            write_ret = true;
        }
    }
    if (write_ret)
    {
        zh_runtime_get_device_health()[g_focus_printer_ip] = 1;
    }
    else
    {
        zh_runtime_get_device_health()[g_focus_printer_ip] = 2;
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

zh_printer_dev::zh_printer_dev(const std::string &_ip) :m_ip(_ip), vl(_ip, ZH_PRINTER_PORT)
{
}
bool zh_printer_dev::print_string(const std::string &_content)
{
    pthread_mutex_lock(&g_plock);
    g_ser_fd = open(vl.get_pts().c_str(), O_RDWR);
    g_focus_printer_ip = m_ip;
    InitializePrint();
    SelChineseChar();
    Set_ChineseCode(2);
    unsigned char buff[2048];
    SetCharacterSize(1, 1);
    Sel_Align_Way(1);
    strcpy((char *)buff, getenv("OEM_NAME"));
    strcat((char *)buff, "\n称重单");
    Print_ASCII(buff);
    Sel_Align_Way(0);
    SetCharacterSize(0, 0);
    print_And_Line();
    strcpy((char *)buff, _content.c_str());
    Print_ASCII(buff);
    print_And_Line();
    print_And_Line();
    strcpy((char *)buff,"卓创智汇\n自动称重系统\n");
    Print_ASCII(buff);
    close(g_ser_fd);
    g_focus_printer_ip = "";
    g_ser_fd = -1;
    pthread_mutex_unlock(&g_plock);
    return true;
}
bool zh_printer_dev::print_qr(const std::string &_qr_code)
{
    pthread_mutex_lock(&g_plock);
    g_ser_fd = open(vl.get_pts().c_str(), O_RDWR);
    g_focus_printer_ip = m_ip;
    InitializePrint();
    Set_QRcodeMode(8);
    Set_QRCodeAdjuLevel(0x49);								/* 设置二维码的纠错水平 */
	Set_QRCodeBuffer(_qr_code.length(), (unsigned char *)(_qr_code.c_str()));	/* 传输数据至编码缓存 */
	Sel_Align_Way(0x01);									/* 居中对齐 */
	PrintQRCode();
    print_And_Line();
    print_And_Line();
    print_And_Line();
    close(g_ser_fd);
    g_ser_fd = -1;
    g_focus_printer_ip = "";
    pthread_mutex_unlock(&g_plock);
    return true;
}