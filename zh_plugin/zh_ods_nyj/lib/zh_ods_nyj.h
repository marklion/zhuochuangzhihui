#include <string>
#include <list>
#include "../../../zh_pub/zh_cpp_pub/zh_plugin_conf.h"
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
#define PLUGIN_CONF_FILE "/plugin/zh_ods_nyj/conf/plugin.json"

bool proc_p_weight(const std::string &_vehicle_number);
bool proc_m_weight(const std::string &_vehicle_number);