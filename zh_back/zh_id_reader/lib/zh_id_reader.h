#if !defined(_ZH_ID_READER_H_)
#define _ZH_ID_READER_H_

extern "C"
{
#include "libtermb.h"
}
#include "../../zh_tdf/tdf_include.h"

std::string read_id_no(const std::string &ip, unsigned short port);

#endif // _ZH_ID_READER_H_
