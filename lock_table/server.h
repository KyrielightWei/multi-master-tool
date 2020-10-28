/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/server.h
 */
 
#include <brpc/server.h>
#include <brpc/channel.h>
#include "lock_table.pb.h"
#include "lock_inform.pb.h"
#include <iostream>
#include <string>
#include <ctime>
#include <map>
#include <mutex>
#include <set>
#include <queue>
#include <utility>
#include <thread>
#include <fstream>

#define DEBUG 1

#define NOLOCK 0
#define READ 1
#define WRITE 2

//server which handle lock table 
class LOCKTableImpl : public lock_table::LOCKService {
    private:
        std::mutex table_lock;
        std::map<std::string,int> lock_type;
        std::map<std::string,std::set<std::string>> client_set;
        std::map<std::string,std::queue<std::pair<std::string,int>>> client_wait;

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

// The server informs the client that it has acquired the lock
class InformSender
{
    private:
    std::string remote_addr;
    brpc::Channel channel;
    lock_inform::InformService_Stub * stub;

    public:
    int set_remote_addr(std::string addr);
    int init();
    int send_mess(std::string str);
};

void HandleLockInformationResponse(brpc::Controller* cntl,lock_inform::InformResponse* response,InformSender * InformSender);