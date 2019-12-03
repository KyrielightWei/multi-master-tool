/***
 * Common Network transmission handle
 * Basic Class
*/
#ifndef NETWORK_HANDLE_HEADER

#define NETWORK_HANDLE_HEADER

#include<string>
#include <iostream>

typedef int EventType;
typedef void (*NetworkHandle_CB)(EventType event, void * arg);

class NetworkHandle
{
    public:
    virtual bool init_handle()=0;
    //init_clientHandle
    virtual bool free_handle()=0;

    virtual bool is_free()=0;
    virtual bool is_init()=0;
   
    virtual int get_connection_id()=0;
    virtual int get_connection_count()=0;

    virtual bool send(const int id,const char * send_bytes,const int send_size)=0;
    
    virtual bool wait_recive(const int id,char * recive_bytes,int * recive_size)=0;
    
    virtual int get_recive_buffer_length(const int id)=0;
    
    virtual int get_listen_connection_count()=0;
    virtual void get_listen_connection_array(int * array)=0;

    virtual void set_event_callback(NetworkHandle_CB cb,void * arg)=0;

    static const EventType EVENT_RECIVE = 1;
};


#endif // !NETWORK_HANDLE_HEADER