#if !defined(_STATE_MACHINE_CONTROL_H_)
#define _STATE_MACHINE_CONTROL_H_

#include "../../utils/local_utils.h"

std::string sm_control_take_picture(const std::string _name, bool _is_enter);
std::string sm_control_get_record(const std::string _name, bool _is_enter, const std::string &_begin_date, const std::string &_end_date);

#endif // _STATE_MACHINE_CONTROL_H_
