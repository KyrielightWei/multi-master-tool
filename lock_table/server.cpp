/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/server.cpp
 */

#include "server.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#if DEBUG
    #include <fstream>
    std::mutex debug_lock;
    int debug_num=0;
#endif

//log configure
auto my_logger = spdlog::basic_logger_mt("SERVER", "server.log");;

/**
***LockTableServer
***server which handle lock table
***/

void back_to_client(std::string addr,std::string c_c_addr,std::string table_page,int type)
{
    InformSender s_c;
    s_c.set_remote_addr(addr);
    s_c.init();
    s_c.send_mess(c_c_addr+","+table_page+","+std::to_string(type));
}


void LOCKTableImpl::LOCKTable(google::protobuf::RpcController* cntl_base, const lock_table::LOCKRequest* request, lock_table::LOCKResponse* response, google::protobuf::Closure* done)
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    std::lock_guard<std::mutex> guard(table_lock);
        
    time_t now = time(0);
    char* dt = ctime(&now);
        
    std::cout<<"Received request from " << cntl->remote_side() << " to " << cntl->local_side()<< "-- message:" << request->message()<<" ,time:"<<dt<<std::endl;

    std::string str=request->message();
    size_t pos=str.find(",");std::string mess=str.substr(0,pos);
    str=str.substr(pos+1,str.size());pos=str.find(",");std::string tableid=str.substr(0,pos);
    str=str.substr(pos+1,str.size());pos=str.find(",");std::string pageid=str.substr(0,pos);
    std::string c_s_port=str.substr(pos+1,str.size());
    std::string table_page = tableid+"-"+pageid;
    std::string client_addr=butil::endpoint2str(cntl->remote_side()).c_str();
    std::string addr=client_addr+","+c_s_port;
    std::pair<std::string,int> addr_and_type;
        
    if(mess=="read"){
        if(lock_type[table_page]==NOLOCK||lock_type[table_page]==READ){
            lock_type[table_page]=READ;
            client_set[table_page].insert(client_addr);
            response->set_message("ok");
        }
        else if(lock_type[table_page]==WRITE){
            addr_and_type=make_pair(addr,READ);
            client_wait[table_page].push(addr_and_type);
            response->set_message("wait");
        }
    }

    else if(mess=="write"){
        if(lock_type[table_page]==NOLOCK){
            lock_type[table_page]=WRITE;
            client_set[table_page].insert(client_addr);
            response->set_message("ok");
        }
        else if(lock_type[table_page]==READ||lock_type[table_page]==WRITE){
            addr_and_type=make_pair(addr,WRITE);
            client_wait[table_page].push(addr_and_type);
            response->set_message("wait");
        }
    }

    else if(mess=="realse"){
        client_set[table_page].erase(client_addr);
        if(client_set[table_page].size()==0&&client_wait[table_page].size()==0){
            lock_type[table_page]=NOLOCK;
        }
        else if(client_set[table_page].size()==0){
            str=client_wait[table_page].front().first;
            pos=str.find(":");std::string c_s_addr=str.substr(0,pos)+":";
            pos=str.find(",");std::string client_address=str.substr(0,pos);
            c_s_addr=c_s_addr+str.substr(pos+1,str.size());

            client_set[table_page].insert(client_address);
            lock_type[table_page]=client_wait[table_page].front().second;
            client_wait[table_page].pop();

            //talk to client
            std::thread th[10000];
            int i=0;
            th[i] = std::thread(back_to_client, c_s_addr,client_address,table_page,lock_type[table_page]);
            th[i++].detach();

            if(lock_type[table_page]==READ){
                while(client_wait[table_page].front().second==READ){
                    str=client_wait[table_page].front().first;
                    pos=str.find(":");c_s_addr=str.substr(0,pos)+":";
                    pos=str.find(",");client_address=str.substr(0,pos);
                    c_s_addr=c_s_addr+str.substr(pos+1,str.size());

                    client_set[table_page].insert(client_address);
                    client_wait[table_page].pop();  

                    th[i] = std::thread(back_to_client, c_s_addr,client_address,table_page,lock_type[table_page]);
                    th[i++].detach();              
                }
            }
        }
        response->set_message("ok");
    }

#if DEBUG
    std::map<std::string,int>::iterator mapit;
    std::map<std::string,std::set<std::string>>::iterator clientit;
    std::set<std::string> cliset;
    std::set<std::string>::iterator it;
    std::string debug_mess="receive : "+request->message();
    for(mapit=lock_type.begin();mapit!=lock_type.end();mapit++)
    {
        debug_mess+=" table/page: "+mapit->first+" lock:"+std::to_string(mapit->second)+" client:";
        for(it=client_set[mapit->first].begin();it!=client_set[mapit->first].end();it++)
        {
            debug_mess+=*it+",";
        }
        debug_mess+=" ** wait list: ";
        std::queue<std::pair<std::string,int>> quetmp=client_wait[mapit->first];
        while(!quetmp.empty())
        {
            debug_mess+=quetmp.front().first+",";
            quetmp.pop();
        }
    }
    debug_lock.lock();
    debug_num++;
    debug_mess+=" ** sum request= "+std::to_string(debug_num);
    debug_lock.unlock();
    my_logger->debug(debug_mess);
#endif
}


int LockTableServer::init()
{
    my_logger = spdlog::basic_logger_mt("SERVER-"+std::to_string(port), "server.log");
    #if DEBUG
    my_logger->set_level(spdlog::level::debug);
    #endif
    my_logger->flush_on(spdlog::level::debug);

    if (server.AddService(&lock_table_service_impl, brpc::SERVER_OWNS_SERVICE) != 0) {
        my_logger->error("Fail to add service");
        return -1;
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = -1;
    if (server.Start(port, &options) != 0) {
        my_logger->error("Fail to start Server");
        return -1;
    }
    my_logger->info("Server start");
}

int LockTableServer::run()
{
    server.RunUntilAskedToQuit();
}

/**
***InformSender
***The server informs the client that it has acquired the lock
***/

int InformSender::init()
{
    brpc::ChannelOptions options;
    options.protocol = brpc::PROTOCOL_BAIDU_STD;
    options.connection_type = brpc::CONNECTION_TYPE_SINGLE;
    if (channel.Init(remote_addr.c_str(), &options) != 0) {
        my_logger->error("Fail to initialize channel to send inform to client");
        return -1;
    }
}

int InformSender::set_remote_addr(std::string addr)
{
    remote_addr=addr;
}

int InformSender::send_mess(std::string str)
{
    stub=new lock_inform::InformService_Stub(&channel);
    lock_inform::InformRequest request;
    lock_inform::InformResponse* response=new lock_inform::InformResponse();
    brpc::Controller* cntl = new brpc::Controller();

    request.set_message(str);

    google::protobuf::Closure* done = brpc::NewCallback(&HandleLockInformationResponse,cntl,response,this);
    stub->LockInform(cntl, &request, response, done);
}

void HandleLockInformationResponse(brpc::Controller* cntl,lock_inform::InformResponse* response,InformSender * InformSender)
{
    std::unique_ptr<brpc::Controller> cntl_guard(cntl);
    std::unique_ptr<lock_inform::InformResponse> response_guard(response);
    if(cntl->Failed()){
        my_logger->error(cntl->ErrorText());
        return;
    }
    #if DEBUG
        my_logger->debug(response->message());
    #endif
}