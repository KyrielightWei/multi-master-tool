
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
    int listen_array[100];
    char temp_buf[50];
    while(1)
    {
        memset(temp_buf,0,sizeof(char)*50);
        sleep(10);
        cout << "listen count " << lib.get_listen_connection_count() << std::endl;
        lib.get_listen_connection_array(listen_array);
        lib.wait_recive(listen_array[0],temp_buf);
        cout << "recive buffer length " << lib.get_recive_buffer_length(listen_array[0])<< std::endl;
        cout << temp_buf << std::endl;
    };
    return 0;
}
