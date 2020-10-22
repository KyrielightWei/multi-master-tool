/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/server.cpp
 */

#include "server.h"
#include "client.h"
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
#include <thread>
//debug
#include <fstream>
//server which handle lock table (in server-side)

void back_to_client(std::string addr,std::string c_c_addr,std::string table_page,int type)
{
    server_s_client s_c;
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

//****************debug
    std::ofstream ofs;
    ofs.open("./map.txt",std::ios::app);
    std::map<std::string,int>::iterator mapit;
    std::map<std::string,std::set<std::string>>::iterator clientit;
    std::set<std::string> cliset;
    std::set<std::string>::iterator it;
    ofs<<std::endl<<"---------------"<<std::endl;
    ofs<<request->message()<<std::endl;
    for(mapit=lock_type.begin();mapit!=lock_type.end();mapit++)
    {
        ofs<<"table/page:"<<mapit->first<<" lock:"<<mapit->second<<" client:";
        for(it=client_set[mapit->first].begin();it!=client_set[mapit->first].end();it++)
        {
            ofs<<*it<<",";
        }
        ofs<<" /// wait list:";
        std::queue<std::pair<std::string,int>> quetmp=client_wait[mapit->first];
        while(!quetmp.empty())
        {
            ofs<<quetmp.front().first<<",";
            quetmp.pop();
        }
        ofs<<std::endl;
    }
    ofs<<"---------------"<<std::endl;
    ofs.close();
}


int LockTableServer::init()
{
    if (server.AddService(&lock_table_service_impl, brpc::SERVER_OWNS_SERVICE) != 0) {
        std::cout << "Fail to add service"<<std::endl;
        return -1;
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = -1;
    if (server.Start(port, &options) != 0) {
        std::cout << "Fail to start Server"<<std::endl;
        return -1;
    }
    std::cout<<"Server start"<<std::endl;
}

int LockTableServer::run()
{
    server.RunUntilAskedToQuit();
}


//server which receive the request that waited lock can be obtained (in client-side)

void STOCImpl::server_to_client(google::protobuf::RpcController* cntl_base, const servertoclient::STOCRequest* request, servertoclient::STOCResponse* response, google::protobuf::Closure* done) 
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
    response->set_message("i know");
}

int client_server::init()
{
    if (cli_s_server.AddService(&client_s_server, brpc::SERVER_OWNS_SERVICE) != 0) {
        std::cout << "Fail to add service in client"<<std::endl;
        return -1;
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = -1;
    if (cli_s_server.Start(port, &options) != 0) {
        std::cout << "Fail to start Server in client"<<std::endl;
        return -1;
    }
    std::cout<<"Server start in client"<<std::endl;
}

int client_server::run()
{
    std::cout<<"client's server is run"<<std::endl;
    cli_s_server.RunUntilAskedToQuit();
}

int client_server::get_port()
{
    return port;
}

int client_server::set_port(int setport)
{ 
    port=setport;
}