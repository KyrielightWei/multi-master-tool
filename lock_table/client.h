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
#include "lock_table.pb.h"
#include "STOC.pb.h"
#include <iostream>
#include <ctime>
#include <string>
#include <random>
#include <mutex>
#include <stdlib.h>
#include <condition_variable>
#include <map>

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

// client which send lock request to server (in client-side)
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

// client which inform the request side lock is taken (in server-side)
class server_s_client
{
    private:
    std::string remote_addr;
    brpc::Channel channel;
    servertoclient::STOCService_Stub * stub;

    public:
    int set_remote_addr(std::string addr);
    int init();
    int send_mess(std::string str);
};

void STOCResponse(brpc::Controller* cntl,servertoclient::STOCResponse* response,server_s_client * server_s_client);
