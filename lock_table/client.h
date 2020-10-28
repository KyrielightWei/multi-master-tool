/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/client.h
 */
#include <butil/time.h>
#include <brpc/channel.h>
#include <brpc/server.h>
#include "lock_table.pb.h"
#include "lock_inform.pb.h"
#include <iostream>
#include <ctime>
#include <string>
#include <random>
#include <mutex>
#include <stdlib.h>
#include <condition_variable>
#include <map>
#include <thread>
#include <chrono>

#define DEBUG 1

#define TableID int
#define PageID int

//condition variable for thread communication
extern std::map<std::string,std::map<std::string,std::pair<std::mutex,std::condition_variable>>> shared_cv;

struct mess
{
    TableID table_id;
    PageID page_id;
    std::string request_type;
    int c_s_port;
};

// The client send lock request to the server
class RemoteLockTable
{
    private:
    mess send_message;
    std::string server_addr="10.11.6.116:60066";
    brpc::Channel channel;
    lock_table::LOCKService_Stub * stub;
    bool send_success=false;

    public:    
    std::mutex result_lock;
    std::condition_variable result_cond;
    
    int init();
    int send_request();
    int set_message(TableID table_id,PageID page_id,std::string request_type,int c_s_port);
    mess get_message();
    bool get_response();
    int set_result(bool res);
};

void HandleLOCKTableResponse(brpc::Controller* cntl,lock_table::LOCKResponse* response,RemoteLockTable * remote_lock_table);

//The client is notified that it has acquired the lock and does not need to block any more
class LockInformImpl : public lock_inform::InformService {
    private:
        std::mutex lock;

    public:
        LockInformImpl() {};
        virtual ~LockInformImpl() {};
        virtual void LockInform(google::protobuf::RpcController* cntl_base, const lock_inform::InformRequest* request, lock_inform::InformResponse* response, google::protobuf::Closure* done);
};

class InformReceiver
{
    private:
    int port;
    brpc::Server server;
    LockInformImpl inform_server;

    public:
    int init();
    int run();
    int get_port();
    int set_port(int setport);
};
