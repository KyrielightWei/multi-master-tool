/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/server.h
 */
 
#include <brpc/server.h>
#include "lock_table.pb.h"
#include "STOC.pb.h"
#include <iostream>
#include <string>
#include <ctime>
#include <map>
#include <mutex>
#include <set>
#include <queue>
#include <utility>

#define NOLOCK 0
#define READ 1
#define WRITE 2

//server which handle lock table (in server-side)
class LOCKTableImpl : public lock_table::LOCKService {
    private:
        std::mutex table_lock;
        std::map<std::string,int> lock_type;
        std::map<std::string,std::set<std::string>> client_set;
        std::map<std::string,std::queue<std::pair<std::string,int>>> client_wait;
        std::pair<std::string,int> addr_and_type;

    public:
        LOCKTableImpl() {};
        virtual ~LOCKTableImpl() {};
        virtual void LOCKTable(google::protobuf::RpcController* cntl_base, const lock_table::LOCKRequest* request, lock_table::LOCKResponse* response, google::protobuf::Closure* done) ;
};

class LockTableServer
{
    private:
    int port=60066;
    brpc::Server server;
    LOCKTableImpl lock_table_service_impl;

    public:
    int init();
    int run();
};

//server which receive the request that waited lock can be obtained (in client-side)
class STOCImpl : public servertoclient::STOCService {
    private:
        std::mutex lock;

    public:
        STOCImpl() {};
        virtual ~STOCImpl() {};
        virtual void server_to_client(google::protobuf::RpcController* cntl_base, const servertoclient::STOCRequest* request, servertoclient::STOCResponse* response, google::protobuf::Closure* done);
};

class client_server
{
    private:
    int port;
    brpc::Server cli_s_server;
    STOCImpl client_s_server;

    public:
    int init();
    int run();
    int get_port();
    int set_port(int setport);
};