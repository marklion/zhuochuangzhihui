#include "zh_printer.h"
#include <unistd.h>
#include "../../zh_rpc_server/zh_rpc_util.h"
#include "../../zh_tdf/tdf_include.h"
#include "DP_Print_inc.h"
#include <iconv.h>
static int g_ser_fd = -1;
static std::string g_focus_printer_ip;
int connect_to_device_tcp_server(const std::string &_ip, unsigned short _port)
{
    int ret = -1;
    sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(_port),
        .sin_addr = {.s_addr = inet_addr(_ip.c_str())},
    };
    auto socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd >= 0)
    {
        if (0 == connect(socket_fd, (sockaddr *)&server_addr, sizeof(server_addr)))
        {
            ret = socket_fd;
        }
        else
        {
            perror("failed to connect server");
            close(socket_fd);
        }
    }
    else
    {
        perror("failed to open socket fd");
    }
    return ret;
}
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
        if (1 == send(g_ser_fd, &Send_Dat, 1, 0))
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
    return '0';
}

zh_printer_dev::zh_printer_dev(const std::string &_ip) :m_ip(_ip), vl(_ip, ZH_PRINTER_PORT)
{
}
bool zh_printer_dev::print_string(const std::string &_content, const std::string &_qr_code)
{
    pthread_mutex_lock(&g_plock);
    g_ser_fd = connect_to_device_tcp_server(m_ip, ZH_PRINTER_PORT);
    g_focus_printer_ip = m_ip;
    InitializePrint();
    SelChineseChar();
    char buff[2048];
    SetCharacterSize(0, 0);
    Set_LeftSpaceNum(12, 0);
    Sel_Align_Way(0);
    strcpy((char *)buff, getenv("OEM_NAME"));
    strcat((char *)buff, "称重单\n");
    Print_ASCII(utf2gbk(buff).c_str());
    print_And_Line();
    strcpy((char *)buff, _content.c_str());
    Print_ASCII(utf2gbk(buff).c_str());
    if (_qr_code.length() > 0)
    {
        Set_QRcodeMode(8);
        Set_QRCodeAdjuLevel(0x31); /* 设置二维码的纠错水平 */
        Sel_Align_Way(1);
        Set_QRCodeBuffer(_qr_code.length(), (unsigned char *)(_qr_code.c_str())); /* 传输数据至编码缓存 */
        PrintQRCode();
    }
    print_And_Line();
    Sel_Align_Way(0);
    strcpy((char *)buff, "卓创智汇\n自动称重系统\n");
    Print_ASCII(utf2gbk(buff).c_str());
    print_And_Line();

    UART_SendByte(0x1D);
    UART_SendByte(0x56);
    UART_SendByte(66);
    UART_SendByte(15);
    close(g_ser_fd);
    g_focus_printer_ip = "";
    g_ser_fd = -1;
    pthread_mutex_unlock(&g_plock);
    return true;
}