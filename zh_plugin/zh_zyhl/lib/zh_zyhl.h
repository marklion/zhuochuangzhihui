#if !defined(_ZYHL_LIB_H_)
#define _ZYHL_LIB_H_
#include <string>
#include <list>
#include "../../../zh_pub/zh_cpp_pub/CJsonObject.hpp"
#include "../../../zh_pub/zh_cpp_pub/Base64.h"
#include "../../../zh_back/zh_tdf/tdf_include.h"
#include "../../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../../zh_pub/zh_rpc_base/gen_code/cpp/vehicle_order_center.h"
#include <thrift/transport/THttpClient.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/protocol/TMultiplexedProtocol.h>

#define THR_DEF_CIENT(x) x##Client *client = nullptr
#define THR_CONNECT(x) std::shared_ptr<TTransport> transport(new THttpClient("localhost", 8123, "/zh_rpc"));std::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport)); transport->open();  std::shared_ptr<TMultiplexedProtocol> mp(new TMultiplexedProtocol(protocol, #x)); client = new x##Client(mp)
#define TRH_CLOSE() transport->close()

void fetch_plan_from_zyhl(const std::string &_date);
std::string util_get_datestring(time_t _time, bool need_offset = true);
bool push_vehicle_enter(const std::string &_vehicle_number, double _xxx);
bool push_vehicle_weight(const std::string &_vehicle_number, double _weight);

std::string send_file_to_zyhl(const std::string &_id, const std::string &_path);
void get_zip_ticket(const std::string &_begin_date, const std::string &_end_date, const std::string &_trans_comapny_name);
void test_strim();
#endif // _ZYHL_LIB_H_
