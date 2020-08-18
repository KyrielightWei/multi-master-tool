/*
 * @Author: liu
 * @Date: 2020-07-23 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/client.cpp
 */

#include <gflags/gflags.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include "lock_table.pb.h"
#include <iostream>
#include <ctime>
#include <string>
#include <random>
#include <mutex>
#include <brpc/retry_policy.h>
#include <stdlib.h>
using namespace std;

#define TableID int
#define PageID int

DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "10.11.6.116:60006", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 50000, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 5, "Max retries(not including the first RPC)"); 
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");
DEFINE_string(request_type,"","read/write/realse");
 
class MyRetryPolicy : public brpc::RetryPolicy {
public:
    bool DoRetry(const brpc::Controller* cntl) const {
        if (cntl->ErrorCode() == 2001 && cntl->ErrorText().substr(cntl->ErrorText().length()-13,13)=="lock conflict") {
            sleep(5);//wait for next retry
            return true;
        }
        return brpc::DefaultRetryPolicy()->DoRetry(cntl);
    }
};
 
bool retry=1;
void HandleLOCKTableResponse(brpc::Controller* cntl,lock_table::LOCKResponse* response)
{
    unique_ptr<brpc::Controller> cntl_guard(cntl);
    unique_ptr<lock_table::LOCKResponse> response_guard(response);
    
    if (cntl->Failed()) {
        cout << cntl->ErrorText()<<endl;
        return;
    }
    cout<<"Received : "<<response->message()<<endl;

}

int main(int argc, char* argv[]) {

    google::ParseCommandLineFlags(&argc, &argv, true);

    logging::LoggingSettings log_setting;
    log_setting.logging_dest=logging::LOG_TO_FILE;
    string log_path="./lock_table_client.log";
    log_setting.log_file=log_path.c_str();
    logging::InitLogging(log_setting);
    brpc::Channel channel;

    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms;
    options.max_retry = FLAGS_max_retry;
    static MyRetryPolicy g_my_retry_policy;
    options.retry_policy = &g_my_retry_policy;
    if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        cout << "Fail to initialize channel"<<endl;;
        return -1;
    }

    lock_table::LOCKService_Stub stub(&channel);

    int log_id = 0;
    TableID table_id;
    PageID page_id;
    string request_type;

    // for(int i=0;i<1000;i++)
    while(cin>>table_id>>page_id>>request_type)
    {
        lock_table::LOCKRequest request;
        lock_table::LOCKResponse* response=new lock_table::LOCKResponse();
        brpc::Controller* cntl = new brpc::Controller();
        cntl->set_log_id(log_id ++); 
        cntl->set_max_retry(3);

        //rand command
        // string a[3]={"read","write","realse"};
        // table_id=rand()%10;
        // page_id = rand()%4;
        // request_type =a[rand()%3];
        string message= request_type+"," + to_string(table_id)+","+to_string(page_id);
        request.set_message(message);
    
        //callback
        google::protobuf::Closure* done = brpc::NewCallback(&HandleLOCKTableResponse,cntl,response);
        stub.LOCKTable(cntl, &request, response, done);
        // usleep(FLAGS_interval_ms * 1000L);
        sleep(3);
    }
    sleep(120);
    cout<<"Client is going to quit"<<endl;
    return 0;
}
