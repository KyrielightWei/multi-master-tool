/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/client.cpp
 */

#include "client.h"
#include <gflags/gflags.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include "lock_table.pb.h"
#include "STOC.pb.h"
#include <iostream>
#include <ctime>
#include <string>
#include <mutex>
#include <stdlib.h>
#include <condition_variable>

std::map<std::string ,std::map<std::string ,std::pair<std::mutex ,std::condition_variable>>> shared_cv;

// *****RemoteLockTable
// client which send lock request to server (in client-side)

int RemoteLockTable::init()
{
    brpc::ChannelOptions options;
    options.protocol = brpc::PROTOCOL_BAIDU_STD;
    options.connection_type = brpc::CONNECTION_TYPE_SINGLE;
    if (channel.Init(server_addr.c_str(), &options) != 0) {
        std::cout << "Fail to initialize channel"<<std::endl;
        return -1;
    }
}

int RemoteLockTable::send_request()
{
    stub=new lock_table::LOCKService_Stub(&channel);
    lock_table::LOCKRequest request;
    lock_table::LOCKResponse* response=new lock_table::LOCKResponse();
    brpc::Controller* cntl = new brpc::Controller();

    std::string message= send_message.request_type+"," + std::to_string(send_message.table_id)+","+std::to_string(send_message.page_id)+","+std::to_string(send_message.c_s_port);
    request.set_message(message);

    google::protobuf::Closure* done = brpc::NewCallback(&HandleLOCKTableResponse,cntl,response,this);
    stub->LOCKTable(cntl, &request, response, done);
}

int RemoteLockTable::set_message(TableID table_id,PageID page_id,std::string request_type,int c_s_port)
{
    send_message.table_id=table_id;
    send_message.page_id=page_id;
    send_message.request_type=request_type;
    send_message.c_s_port=c_s_port;
}

mess RemoteLockTable::get_message()
{
    mess _message;
    _message.table_id=send_message.table_id;
    _message.page_id=send_message.page_id;
    _message.request_type=send_message.request_type;
    _message.c_s_port=send_message.c_s_port;
    return _message;
}

void HandleLOCKTableResponse(brpc::Controller* cntl,lock_table::LOCKResponse* response,RemoteLockTable * remote_lock_table)
{
    remote_lock_table->set_result(false);
    std::unique_ptr<brpc::Controller> cntl_guard(cntl);
    std::unique_ptr<lock_table::LOCKResponse> response_guard(response);

    if(cntl->Failed()){
        remote_lock_table->send_request();
        std::cout << cntl->ErrorText()<<std::endl;
        return;
    }

    std::cout<<"Received lock request result : "<<response->message()<<std::endl;
    if(response->message()=="wait"){
        mess sm=remote_lock_table->get_message();
        std::string req_type=(sm.request_type=="write")?"2":"1";
        std::unique_lock<std::mutex> locker(shared_cv[butil::endpoint2str(cntl->local_side()).c_str()][std::to_string(sm.table_id)+"-"+std::to_string(sm.page_id)+","+req_type].first);
        //wait lock 
        shared_cv[butil::endpoint2str(cntl->local_side()).c_str()][std::to_string(sm.table_id)+"-"+std::to_string(sm.page_id)+","+req_type].second.wait(locker);
        locker.unlock();
    }
    //get the lock
    remote_lock_table->set_result(true);
    std::unique_lock<std::mutex> res_locker(remote_lock_table->result_lock);
    res_locker.unlock();
    remote_lock_table->result_cond.notify_one();
}

bool RemoteLockTable::get_response()
{
    //response until get the lock
    std::unique_lock<std::mutex> res_locker(result_lock);
    result_cond.wait(res_locker);
    res_locker.unlock();
    return send_success;
}

int RemoteLockTable::set_result(bool res)
{
    send_success=res;
}

// *****server_s_client
// client which inform the request side lock is taken (in server-side)

int server_s_client::init()
{
    brpc::ChannelOptions options;
    options.protocol = brpc::PROTOCOL_BAIDU_STD;
    options.connection_type = brpc::CONNECTION_TYPE_SINGLE;
    if (channel.Init(remote_addr.c_str(), &options) != 0) {
        std::cout << "Fail to initialize channel"<<std::endl;
        return -1;
    }
}

int server_s_client::set_remote_addr(std::string addr)
{
    remote_addr=addr;
}

int server_s_client::send_mess(std::string str)
{
    stub=new servertoclient::STOCService_Stub(&channel);
    servertoclient::STOCRequest request;
    servertoclient::STOCResponse* response=new servertoclient::STOCResponse();
    brpc::Controller* cntl = new brpc::Controller();

    request.set_message(str);

    google::protobuf::Closure* done = brpc::NewCallback(&STOCResponse,cntl,response,this);
    stub->server_to_client(cntl, &request, response, done);
}

void STOCResponse(brpc::Controller* cntl,servertoclient::STOCResponse* response,server_s_client * server_s_client)
{
    std::unique_ptr<brpc::Controller> cntl_guard(cntl);
    std::unique_ptr<servertoclient::STOCResponse> response_guard(response);
    if(cntl->Failed()){
        std::cout << cntl->ErrorText()<<std::endl;
        return;
    }
    std::cout<<"Received : "<<response->message()<<std::endl;
}