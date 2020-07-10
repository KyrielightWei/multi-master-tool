/*
 * @Author: liu
 * @Date: 2020-07-09 09:30
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: 
 */

#include <gflags/gflags.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include "id.pb.h"
#include <iostream>
using namespace std;

// DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "10.11.6.120:60006", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 1000, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 5, "Max retries(not including the first RPC)"); 
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");

int main(int argc, char* argv[]) {

    // Parse gflags
    google::ParseCommandLineFlags(&argc, &argv, true);

    //log
    logging::LoggingSettings log_setting;
    log_setting.logging_dest=logging::LOG_TO_FILE;
    string log_path="./id_client.log";
    log_setting.log_file=log_path.c_str();
    logging::InitLogging(log_setting);
    // A Channel represents a communication line to a Server. Notice that 
    // Channel is thread-safe and can be shared by all threads in your program.
    brpc::Channel channel;

    // Initialize the channel, NULL means using default options.
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
    options.max_retry = FLAGS_max_retry;
    if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        cout << "Fail to initialize channel"<<endl;;
        return -1;
    }

    // Normally, you should not call a Channel directly, but instead construct
    // a stub Service wrapping it. stub can be shared by all threads as well.
    IDIncrement::IDService_Stub stub(&channel);

    // Send a request and wait for the response every 1 second.
    int log_id = 0;
    while (!brpc::IsAskedToQuit()) {
        // We will receive response synchronously, safe to put variables
        // on stack.
        IDIncrement::IDRequest request;
        IDIncrement::IDResponse response;
        brpc::Controller cntl;

        request.set_message("request id");
/*!!!!!!!!!!!
cntl.set_timeout_ms(...);
stub.some_method(&cntl, &request, &response, NULL);
*/
        cntl.set_log_id(log_id ++);  // set by user
        // Set attachment which is wired to network directly instead of 
        // being serialized into protobuf messages.
        // cntl.request_attachment().append(FLAGS_attachment);

        // Because `done'(last parameter) is NULL, this function waits until
        // the response comes back or error occurs(including timedout).
        stub.IDInc(&cntl, &request, &response, NULL);
        if (!cntl.Failed()) {
            cout<<"Recive:"<<response.message()<<endl;
        } else {
            cout<<"Client Fail"<<endl;
        }
        usleep(FLAGS_interval_ms * 1000L);
    }

    cout<<"Client is going to quit"<<endl;
    return 0;
}
