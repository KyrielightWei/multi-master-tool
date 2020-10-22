/*
 * @Author: liu
 * @Date: 2020-07-09 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/id-increment/server.cpp
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
using namespace std;

DEFINE_int32(port, 60076, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no read/write operations during the last `idle_timeout_s'");
long long id=1;
map<long long,string> idmap;
mutex id_lock;
int num=0;
// implementation of IDIncre::IDService
class IDIncreImpl : public IDIncrement::IDService {
public:
    IDIncreImpl() {};
    virtual ~IDIncreImpl() {};
    virtual void IDInc(google::protobuf::RpcController* cntl_base, const IDIncrement::IDRequest* request, IDIncrement::IDResponse* response, google::protobuf::Closure* done) {

        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
        //lock id
        lock_guard<mutex> guard(id_lock);
        num++;
        // sleep(1);
        //time
        time_t now = time(0);
        char* dt = ctime(&now);
        
        cout<<"Received request[log_id="<<cntl->log_id()<<"] from " << cntl->remote_side() << " to " << cntl->local_side()<< ": " << request->message()<<" ,allot id: "<<id<<" ,time:"<<dt<<endl;
        response->set_message(to_string(id));
        idmap[id]=butil::endpoint2str(cntl->remote_side()).c_str();
        id++;
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
    // options.max_unwritten_bytes=10;
    options.max_unwritten_bytes=10;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) != 0) {
        cout << "Fail to start Server"<<endl;
        return -1;
    }
    cout<<"Server start"<<endl;

    server.RunUntilAskedToQuit();
    cout<<num<<endl;
    freopen("map.out","w",stdout);
    map<long long,string>::iterator it;
    for(it=idmap.begin();it!=idmap.end();it++)
    {
        cout<<"idmap["<<it->first<<"]="<<it->second<<endl;
    }
    return 0;
}
