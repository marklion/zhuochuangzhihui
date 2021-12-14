#include "zh_printer.h"
#include <unistd.h>
#include "../../zh_rpc_server/zh_rpc_util.h"
#include "../../zh_tdf/tdf_include.h"
#include "DP_Print_inc.h"
static int g_ser_fd = -1;

static pthread_mutex_t g_plock = PTHREAD_MUTEX_INITIALIZER;

void UART_SendByte(unsigned char Send_Dat)
{
    if (-1 != g_ser_fd) {
        write(g_ser_fd, &Send_Dat, 1);
    }
}
unsigned char UART_RecByte(void)
{
    unsigned char ret = 0;
    if (-1 != g_ser_fd)
    {
        read(g_ser_fd, &ret, 1);
    }
}

zh_printer_dev::zh_printer_dev(const std::string &_ip) : vl(_ip, ZH_PRINTER_PORT)
{
}
bool zh_printer_dev::print_string(const std::string &_content)
{
    pthread_mutex_lock(&g_plock);
    g_ser_fd = open(vl.get_pts().c_str(), O_RDWR);
    InitializePrint();
    SelChineseChar();
    Set_ChineseCode(2);
    unsigned char buff[2048];
    strcpy((char *)buff,"卓创智汇");
    Print_ASCII(buff);
    print_And_Line();
    strcpy((char *)buff, _content.c_str());
    Print_ASCII(buff);
    print_And_Line();
    print_And_Line();
    print_And_Line();
    print_And_Line();
    print_And_Line();
    print_And_Line();
    close(g_ser_fd);
    g_ser_fd = -1;
    pthread_mutex_unlock(&g_plock);
    return true;
}
bool zh_printer_dev::print_qr(const std::string &_qr_code)
{
    pthread_mutex_lock(&g_plock);
    g_ser_fd = open(vl.get_pts().c_str(), O_RDWR);
    InitializePrint();
    Set_QRcodeMode(8);
    Set_QRCodeAdjuLevel(0x49);								/* 设置二维码的纠错水平 */
	Set_QRCodeBuffer(_qr_code.length(), (unsigned char *)(_qr_code.c_str()));	/* 传输数据至编码缓存 */
	Sel_Align_Way(0x01);									/* 居中对齐 */
	PrintQRCode();
    print_And_Line();
    print_And_Line();
    print_And_Line();
    print_And_Line();
    print_And_Line();
    print_And_Line();
    close(g_ser_fd);
    g_ser_fd = -1;
    pthread_mutex_unlock(&g_plock);
}