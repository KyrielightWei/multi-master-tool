
#include "network_handle.h"
#include <iostream>
#include <string>
#include "libevent_handle.h"


using namespace std;

int main(int argc,char * argv[])
{

//     struct sockaddr_in sin;
//     memset(&sin,0,sizeof(sin));
//     sin.sin_family = AF_INET;
//     sin.sin_addr.s_addr = inet_addr("127.0.0.1");
//     sin.sin_port = 61544;

//     evutil_socket_t socket_fd = socket(AF_INET,SOCK_STREAM,0);
//    // evutil_make_socket_nonblocking(socket_fd);

//     evutil_socket_t socket_fd2 = socket(AF_INET,SOCK_STREAM,0);
//    // evutil_make_socket_nonblocking(socket_fd2);

//     bind(socket_fd,(const sockaddr *)&sin,sizeof(sin));

//     while(1)
//         listen(socket_fd,10);
    
    LibeventHandle lib;
    cout << "arg = "<< argv[1] << std::endl;
    int port = std::stoi(argv[1]);
    cout << "Local Port = " << port << std::endl;
   
    if(!lib.init_handle(port))
    {
        cout << "init error" << std::endl;
    }

    
    //LibeventHandle::event_loop_run(&lib);
    //int last_count = 
    while(1)
    {
        sleep(10);
        cout <<  "connection_count return :"<< lib.get_connection_count() << std::endl;
        cout <<  "error :"<< errno << std::endl;
    };
    return 0;
}
