#include <stdio.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/THttpServer.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/concurrency/ThreadManager.h>
#include "../zh_pub/zh_rpc_base/gen_code/cpp/idl_types.h"
#include "../zh_pub/zh_rpc_base/gen_code/cpp/system_management.h"
#include "zh_rpc_server/system_management_imp.h"
#include "zh_rpc_server/user_management_imp.h"
#include "zh_rpc_server/contract_management_imp.h"
#include "zh_database/zh_db_config.h"
#include <openssl/sha.h>
#include <openssl/crypto.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

struct prj_permission_config {
    long key;
    std::string name;
    std::string description;
};

void init_user_permissions() 
{
    prj_permission_config config[] = {
        {0, "管理员", "管理员可以配置用户，分配权限和其他所有操作"},
        {1, "操作人员", "操作人员可以进行业务操作"},
        {2, "观察人员", "观察人员仅可以查看业务数据，无法操作业务"},
    };

    for (auto &itr:config)
    {
        auto exist_permission = sqlite_orm::search_record<zh_sql_user_permission>("key == '%ld'", itr.key);
        if (exist_permission)
        {
            exist_permission->description = itr.description;
            exist_permission->name = itr.name;
            exist_permission->update_record();
        }
        else
        {
            zh_sql_user_permission tmp;
            tmp.key = itr.key;
            tmp.name = itr.name;
            tmp.description = itr.description;
            tmp.insert_record();
        }
    }
}

std::string make_plain_2_sha1(const std::string &_plain)
{
    SHA_CTX c;
    unsigned char md[SHA_DIGEST_LENGTH];
    SHA1((unsigned char *)_plain.c_str(), _plain.length(), md);
    SHA1_Init(&c);
    SHA1_Update(&c, _plain.c_str(), _plain.length());
    SHA1_Final(md, &c);
    OPENSSL_cleanse(&c, sizeof(c));
    std::string ret;

    for (size_t i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        char tmp[3] = {0};
        sprintf(tmp, "%02x", md[i]);
        ret.append(tmp);
    }

    return ret;
}

void init_admin_user()
{
    std::string admin_phone = "18911992582";
    std::string admin_name = "admin";
    std::string admin_password = make_plain_2_sha1("password");

    auto admin_permission = sqlite_orm::search_record<zh_sql_user_permission>("key == 0");
    if (admin_permission)
    {
        auto exist_admin = admin_permission->get_children<zh_sql_user_info>("permission", "name == '%s'", admin_name.c_str());
        if (!exist_admin)
        {
            zh_sql_user_info admin;
            admin.name = admin_name;
            admin.phone = admin_phone;
            admin.password = admin_password;
            admin.set_parent(*admin_permission, "permission");
            admin.need_change_password = 1;
            admin.insert_record();
        }
    }
}

int main(int argc, char const *argv[])
{
    init_user_permissions();
    init_admin_user();
    std::shared_ptr<TMultiplexedProcessor> multi_processor(new TMultiplexedProcessor());
    multi_processor->registerProcessor("system_management", std::shared_ptr<TProcessor>(new system_managementProcessor(std::shared_ptr<system_management_handler>(system_management_handler::get_inst()))));
    multi_processor->registerProcessor("user_management", std::shared_ptr<TProcessor>(new user_managementProcessor(std::shared_ptr<user_management_handler>(user_management_handler::get_inst()))));
    multi_processor->registerProcessor("contract_management", std::shared_ptr<TProcessor>(new contract_managementProcessor(std::shared_ptr<contract_management_handler>(contract_management_handler::get_inst()))));

    ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(8123));
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
