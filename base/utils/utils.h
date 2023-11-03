#if !defined(_UTILS_H_)
#define _UTILS_H_
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::string> util_split_string(const std::string &s, const std::string &seperator = ",");
std::string util_join_string(const std::vector<std::string> &_vec, const std::string &_dil = ",");
template <typename ele>
bool util_vector_has(const std::vector<ele> &_vec, const ele &_el)
{
    return std::find(_vec.begin(), _vec.end(), _el) != _vec.end();
}

std::string util_calcu_md5(const std::string &_input);
std::string util_data_hex(const unsigned char *_data, int _length);


#endif // _UTILS_H_
