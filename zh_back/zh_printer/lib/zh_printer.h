#if !defined(_ZH_PRINTER_H_)
#define _ZH_PRINTER_H_

#include <string>
#include "../../zh_vcom/zh_vcom_link.h"

void UART_SendByte(unsigned char Send_Dat);
unsigned char UART_RecByte(void);

class zh_printer_dev
{
public:
    zh_vcom_link vl;
    zh_printer_dev(const std::string &_ip);
    bool print_string(const std::string &_content);
    bool print_qr(const std::string &_qr_code);
};

#endif // _ZH_PRINTER_H_
