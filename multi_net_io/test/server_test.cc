
#include "network_handle.h"
#include <iostream>
#include <string>
#include "libevent_handle.h"


using namespace std;

void callback(EventType event, void * arg)
{
     std::cout << "recive function"  <<std::endl;
    switch (event)
    {
    case LibeventHandle::EVENT_RECIVE:
        std::cout << "recive callback invoke !!"  <<std::endl;   
        break;
    
    default:
        break;
    }

}

int main(int argc,char * argv[])
{
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
    lib.set_event_callback(callback,NULL);
   
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
