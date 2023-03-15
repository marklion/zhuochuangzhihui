#include "zh_printer.h"
#include <unistd.h>
#include "../../zh_rpc_server/zh_rpc_util.h"
#include "../../zh_tdf/tdf_include.h"
#include "DP_Print_inc.h"
#include <iconv.h>
static int g_ser_fd = -1;
static std::string g_focus_printer_ip;

static pthread_mutex_t g_plock = PTHREAD_MUTEX_INITIALIZER;
int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}
//UNICODE码转为GB2312码
int u2g(char *inbuf, int inlen, char *outbuf, int outlen)
{
    return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}
std::string utf2gbk(const char *_gbk)
{
    char in_buff[2048] = {0};
    char out_buff[2048] = {0};
    strcpy(in_buff, _gbk);
    u2g(in_buff, strlen(in_buff), out_buff, sizeof(out_buff));
    return std::string(out_buff);
}
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
    Set_ChineseCode(3);
    char buff[2048];
    SetCharacterSize(1, 1);
    Set_LeftSpaceNum(12, 0);
    Sel_Align_Way(0);
    strcpy((char *)buff, getenv("OEM_NAME"));
    strcat((char *)buff, "\n称重单");
    Print_ASCII(utf2gbk(buff).c_str());
    Sel_Align_Way(0);
    SetCharacterSize(0, 0);
    print_And_Line();
    strcpy((char *)buff, _content.c_str());
    Print_ASCII(utf2gbk(buff).c_str());
    print_And_Line();
    print_And_Line();
    strcpy((char *)buff,"卓创智汇\n自动称重系统\n");
    Print_ASCII(utf2gbk(buff).c_str());

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
void zh_printer_dev::cut_paper()
{
    pthread_mutex_lock(&g_plock);
    g_ser_fd = open(vl.get_pts().c_str(), O_RDWR);
    g_focus_printer_ip = m_ip;
    InitializePrint();
	UART_SendByte(0x1D);
	UART_SendByte(0x56);
	UART_SendByte(66);
	UART_SendByte(10);
    close(g_ser_fd);
    g_ser_fd = -1;
    g_focus_printer_ip = "";
    pthread_mutex_unlock(&g_plock);
}