#include "../../base/include.h"
#include "../lib/rpc_include.h"

int main(int argc, char const *argv[])
{
    std::shared_ptr<TMultiplexedProcessor> multi_processor(new TMultiplexedProcessor());
    multi_processor->registerProcessor("device_management", std::shared_ptr<TProcessor>(new device_managementProcessor(std::shared_ptr<device_management_handler>(new device_management_handler()))));

    ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(8124));
    ::std::shared_ptr<TTransportFactory> transportFactory(new THttpServerTransportFactory());
    ::std::shared_ptr<TProtocolFactory> protocolFactory(new TJSONProtocolFactory());
    std::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(16);
    std::shared_ptr<ThreadFactory> threadFactory(new ThreadFactory());
    threadManager->threadFactory(threadFactory);
    threadManager->start();
    TThreadPoolServer tp_server(multi_processor, serverTransport, transportFactory, protocolFactory, threadManager);
    tp_server.serve();

    return 0;
}