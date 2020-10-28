/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/client.cpp
 */

#include "client.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#if DEBUG
    #include <sys/time.h>
    unsigned long long latency_us=0;
    uint64_t start_request;
    struct timeval tv;
#endif

//log configure
auto my_logger = spdlog::basic_logger_mt("CLIENT", "client.log");

//condition variable for thread communication
std::map<std::string ,std::map<std::string ,std::pair<std::mutex ,std::condition_variable>>> shared_cv;

/**
***RemoteLockTable
***The client send lock request to the server
***/

int RemoteLockTable::init()
{
    brpc::ChannelOptions options;
    options.protocol = brpc::PROTOCOL_BAIDU_STD;
    options.connection_type = brpc::CONNECTION_TYPE_SINGLE;
    if (channel.Init(server_addr.c_str(), &options) != 0) {
        my_logger->error("Fail to initialize channel");
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

    #if DEBUG
        start_request=(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())).count();
    #endif

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
        my_logger->error(cntl->ErrorText());
        return;
    }

    std::cout<<"Received lock request result : "<<response->message()<<std::endl;
    if(response->message()=="wait"){
        mess sm=remote_lock_table->get_message();
        std::string req_type=(sm.request_type=="write")?"2":"1";
        my_logger->info("wait for lock "+std::to_string(sm.table_id)+"-"+std::to_string(sm.page_id)+","+sm.request_type);
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
    my_logger->info(std::to_string(send_message.table_id)+"-"+std::to_string(send_message.page_id)+","+send_message.request_type+" success");
    #if DEBUG  
        latency_us=(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())).count()-start_request;
        my_logger->debug("latency="+std::to_string(latency_us)+" us");
    #endif
    return send_success;
}

int RemoteLockTable::set_result(bool res)
{
    send_success=res;
}

/**
***InformReceiver
***The client is notified that it has acquired the lock and does not need to block any more
***/

void LockInformImpl::LockInform(google::protobuf::RpcController* cntl_base, const lock_inform::InformRequest* request, lock_inform::InformResponse* response, google::protobuf::Closure* done) 
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    std::lock_guard<std::mutex> guard(lock);
        
    time_t now = time(0);
    char* dt = ctime(&now);
        
    std::cout<<"Received lock information from " << cntl->remote_side() << " to " << cntl->local_side()<< "-- message:" << request->message()<<" ,time:"<<dt<<std::endl;

    std::string recive_mess=request->message();
    size_t pos=recive_mess.find(",");std::string talk_client =recive_mess.substr(0,pos);
    std::string table_page_tp=recive_mess.substr(pos+1,recive_mess.size());
    std::cout<<talk_client<<" can get lock "<<table_page_tp<<std::endl;

    std::unique_lock<std::mutex> locker(shared_cv[talk_client][table_page_tp].first);
    locker.unlock();
    shared_cv[talk_client][table_page_tp].second.notify_all();   
    response->set_message("The client "+std::string(butil::endpoint2str(cntl->local_side()).c_str())+" receives the lock "+table_page_tp);
}

int InformReceiver::init()
{
    my_logger = spdlog::basic_logger_mt("CLIENT-"+std::to_string(port), "client.log");
    #if DEBUG
    my_logger->set_level(spdlog::level::debug);
    #endif
    my_logger->flush_on(spdlog::level::debug);

    if (server.AddService(&inform_server, brpc::SERVER_OWNS_SERVICE) != 0) {
        my_logger->error("Fail to add inform receiver in client");
        return -1;
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = -1;
    if (server.Start(port, &options) != 0) {
        my_logger->error("Fail to start inform receiver in client");
        return -1;
    }
    my_logger->info("inform receiver start in client");
}

int InformReceiver::run()
{
    my_logger->info("client's inform receiver is run");
    server.RunUntilAskedToQuit();
}

int InformReceiver::get_port()
{
    return port;
}

int InformReceiver::set_port(int setport)
{ 
    port=setport;
}