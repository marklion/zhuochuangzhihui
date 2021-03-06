#include <thrift/transport/THttpClient.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../../zh_pub/zh_rpc_base/gen_code/cpp/open_api.h"
#include <unistd.h>
#include <iostream>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

int main(int argc, char *argv[])
{
    std::shared_ptr<TTransport> transport(new THttpClient("localhost", 8123, "/zh_rpc"));
    std::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport));
    transport->open();
    std::shared_ptr<TMultiplexedProtocol> mp(new TMultiplexedProtocol(protocol, "open_api"));
    auto client = new open_apiClient(mp);
    auto ret = false;
    auto optstring = "p:i:v:s:q:n:c:S:E:";
    int o = -1;

    std::string scale_name;
    std::string id_no;
    std::string vehicle_number;
    std::string road_ip;
    std::string qr_code;
    std::string nvr_ip;
    int channel_id = 0;
    std::string start_time;
    std::string end_time;

    while (-1 != (o = getopt(argc, argv, optstring)))
    {
        switch (o)
        {
        case 'p':
            road_ip = optarg;
            break;
        case 'i':
            id_no = optarg;
            break;
        case 'v':
            vehicle_number = optarg;
            break;
        case 's':
            scale_name = optarg;
            break;
        case 'q':
            qr_code = optarg;
            break;
        case 'n':
            nvr_ip = optarg;
            break;
        case 'c':
            channel_id = atoi(optarg);
            break;
        case 'S':
            start_time = optarg;
            break;
        case 'E':
            end_time = optarg;
            break;
        default:
            break;
        }
    }

    if (nvr_ip.length() > 0)
    {
        std::string file_name;
        client->get_video(file_name, nvr_ip, channel_id, start_time, end_time);
        std::cout << file_name << std::endl;
    }

    if (road_ip.empty())
    {
        return -1;
    }
    if (scale_name == "")
    {
        if (id_no.length() > 0)
        {
            client->external_trigger_gate_id("", id_no, road_ip);
        }
        if (vehicle_number.length() > 0)
        {
            client->external_trigger_gate_vehicle(road_ip, vehicle_number);
        }
        if (qr_code.length() > 0)
        {
            client->external_trigger_gate_qr(road_ip, qr_code);
        }
    }
    else
    {
        if (id_no.length() > 0)
        {
            client->external_trigger_scale_id(road_ip, id_no, scale_name);
        }
        if (vehicle_number.length() > 0)
        {
            client->external_trigger_scale_vehicle(road_ip, vehicle_number, scale_name);
        }
        if (qr_code.length() > 0)
        {
            client->external_trigger_scale_qr(road_ip, qr_code, scale_name);
        }
    }

    return 0;
}
