#if !defined(_FILE_UP_DOWN_H_)
#define _FILE_UP_DOWN_H_

#include <string>
std::string file_store_content(const std::string &_content, const std::string &_ext_name, bool is_tmp = false);
void file_delete(const std::string &_content);

#endif // _FILE_UP_DOWN_H_
