#ifndef LIBEVENT_HANDLE_HEADER

#define LIBEVENT_HANDLE_HEADER


#include "network_handle.h"
#include <map>

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>

#include <thread>
#include <atomic>
#include <condition_variable>

#define LIBEVENT_HANDLE_DEBUG 1


void connlistener_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * sock, int socklen, void * arg);
void listen_error_cb(struct evconnlistener *, void *);

void default_bufferevent_read_cb(struct bufferevent *bev, void *ctx);
void default_bufferevent_write_cb(struct bufferevent *bev, void *ctx);


struct BevInfor
{
    std::string ip;
    int port;
    struct bufferevent * bev;
    bool is_listen;
};

// void fun();
class LibeventHandle:public NetworkHandle
{
    public:


    bool init_handle(int port);
    virtual bool init_handle()
    {
        init_handle(9999);
    }
    virtual bool free_handle();  
   
    virtual bool send(const char * ip,const int port,const char * send_bytes,int send_size);
    virtual bool wait_recive(const char * ip,const int port,char * recive_bytes,int recive_size);

    int get_connection_id(const char * ip,const int port);

    void get_connection_ip(const int id,char * ip);

    int get_connection_port(const int id);

    int get_connection_count()
    {
        return bev_map.size();
    }

    ~LibeventHandle()
    {
        if(isFree = false)
            free_handle();
    }

    bool has_freed()
    {
        return isFree;
    }

    int try_connect(const char* ip,const int port);
    
    void start_event_base_loop();

    /** thread task **/
    static void event_loop_run(LibeventHandle * lib);
    
    private:
    bool init_listener();
    bool init_bufferevent();
    
    int check_connect(const char* ip,const int port);

    int add_bufferevent_connect(const char* ip,const int port);
    int add_bufferevent_listen(const char* ip,const int port,int socket_fd);
    int remove_buffevent(int id);


    struct event_base *main_base;
    struct evconnlistener *conn_listener;
    
    bool isFree;

    int local_port;

    int max_bev_id;
    std::map<int,BevInfor> bev_map;

    ///std::map<int,BevInfor> 

    std::thread * event_base_thread;


    /****************  friend function  **********************/
    friend void connlistener_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * sock, int socklen, void * arg);
    
    friend void default_bufferevent_read_cb(struct bufferevent *bev, void *ctx);
    friend void default_bufferevent_write_cb(struct bufferevent *bev, void *ctx);

};


#endif // !LIBEVENT_HANDLE_HEADER