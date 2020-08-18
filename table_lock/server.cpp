/*
 * @Author: liu
 * @Date: 2020-07-23 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/table_lock/server.cpp
 */


#include <gflags/gflags.h>
#include <brpc/server.h>
#include <butil/logging.h>
#include "id.pb.h"
#include <iostream>
#include <string>
#include <ctime>
#include <map>
#include <mutex>
#include <set>
using namespace std;

DEFINE_int32(port, 60006, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no read/write operations during the last `idle_timeout_s'");

map<string,int> lock_map;
map<string,set<string>> client_set;
#define Nolock 0
#define Read 1
#define Write 2
mutex table_lock;
int num=0; 

class IDIncreImpl : public IDIncrement::IDService {
public:
    IDIncreImpl() {};
    virtual ~IDIncreImpl() {};
    virtual void IDInc(google::protobuf::RpcController* cntl_base, const IDIncrement::IDRequest* request, IDIncrement::IDResponse* response, google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
        //lock id
        lock_guard<mutex> guard(table_lock);
        
        time_t now = time(0);
        char* dt = ctime(&now);
        
        // idmap[butil::endpoint2str(cntl->remote_side()).c_str()]=id;
        // cout<<"Received request[log_id="<<cntl->log_id()<<"] from " << cntl->remote_side() << " to " << cntl->local_side()
        // << "-- message:" << request->message()<<" tableid:"<<request->table_id()<<" pageid:"<<request->page_id()<<" ,time:"<<dt<<endl;
        cout<<"Received request[log_id="<<cntl->log_id()<<"] from " << cntl->remote_side() << " to " << cntl->local_side()<< "-- message:" << request->message()<<" ,time:"<<dt<<endl;

        string str=request->message();
        size_t pos=str.find(",");string mess=str.substr(0,pos);
        str=str.substr(pos+1,str.size());pos=str.find(",");string tableid=str.substr(0,pos);
        string pageid=str.substr(pos+1,str.size());
        string table_page = tableid+"-"+pageid;
        string client_addr=butil::endpoint2str(cntl->remote_side()).c_str();

        if(mess=="read"){
            if(lock_map[table_page]==Nolock||lock_map[table_page]==Read){
                lock_map[table_page]=Read;
                client_set[table_page].insert(client_addr);
                response->set_message("ok");
            }
            else if(lock_map[table_page]==Write){
                response->set_message("fail");
                cntl->SetFailed("lock conflict");
            }

        }
        else if(mess=="write"){
            if(lock_map[table_page]==Nolock){
                lock_map[table_page]=Write;
                client_set[table_page].insert(client_addr);
                response->set_message("ok");
            }
            else if(lock_map[table_page]==Write||lock_map[table_page]==Read){
                response->set_message("fail");
                cntl->SetFailed("lock conflict");
            }
        }
        else if(mess=="realse"){
            client_set[table_page].erase(client_addr);
            if(client_set[table_page].size()==0){
                lock_map[table_page]=Nolock;
            }
            response->set_message("ok");
        }
        num++;
    }
};

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);

    brpc::Server server;

    logging::LoggingSettings log_setting;
    log_setting.logging_dest=logging::LOG_TO_FILE;
    string log_path="./id_server.log";
    log_setting.log_file=log_path.c_str();
    logging::InitLogging(log_setting);

    IDIncreImpl id_service_impl;

    if (server.AddService(&id_service_impl, brpc::SERVER_OWNS_SERVICE) != 0) {
        cout << "Fail to add service"<<endl;
        return -1;
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) != 0) {
        cout << "Fail to start Server"<<endl;
        return -1;
    }
    cout<<"Server start"<<endl;

    server.RunUntilAskedToQuit();
    
    time_t now = time(0);
    char* dtt = ctime(&now);
    freopen("clientmap.out","w",stdout);
    cout<<dtt<<"-----------------------------------"<<endl;
    cout<<num<<endl;
    map<string,int>::iterator mapit;
    map<string,set<string>>::iterator clientit;
    set<string> cliset;
    set<string>::iterator it;
    for(mapit=lock_map.begin();mapit!=lock_map.end();mapit++)
    {
        cout<<"table/page:"<<mapit->first<<" lock:"<<mapit->second<<" client:";
        for(it=client_set[mapit->first].begin();it!=client_set[mapit->first].end();it++)
        {
            cout<<*it<<",";
        }
        cout<<endl;
    }

    return 0;
}
