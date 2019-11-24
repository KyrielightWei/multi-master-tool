#include "libevent_handle.h"

bool LibeventHandle::init_handle(int port)
{
    main_base=event_base_new();
    if (!main_base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return false;
    }
    local_port = port;

    max_bev_id = 0;
    bev_map.clear();

    init_listener();

    event_base_thread = new std::thread(event_loop_run,this);

    
    isFree=false;

    return true;
}

bool LibeventHandle::free_handle()
{
    if(isFree == true)
        return false;

    event_base_loopexit(main_base,NULL);
    free(event_base_thread);
    evconnlistener_free(conn_listener);
    for (auto it = bev_map.begin(); it != bev_map.end() ; it++)
    {
        struct BevInfor & info = it->second;
        bufferevent_free(info.bev);
    }
    bev_map.clear();
    
    event_base_free(main_base);
    isFree=true;
    return isFree;
}

bool LibeventHandle::init_listener()
{
    struct sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(local_port);
    sin.sin_addr.s_addr = htonl(0);

    conn_listener = evconnlistener_new_bind(main_base,&connlistener_cb,this,
    LEV_OPT_CLOSE_ON_FREE  | LEV_OPT_THREADSAFE | LEV_OPT_REUSEABLE,
    -1,(struct sockaddr*)&sin,sizeof(sin));

    if(!conn_listener)
    {
        #if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl; 
        std::cout << "conn_listener address = "<< conn_listener  << std::endl; 
        #endif // DEBUG
        return false;
    }
    evconnlistener_set_error_cb(conn_listener,listen_error_cb);
    evconnlistener_enable(conn_listener);

    #if LIBEVENT_HANDLE_DEBUG
        std::cout << "evconnlistener_get_fd = " << evconnlistener_get_fd(conn_listener) << std::endl;
    #endif // DEBUG0

    return true;
}

int LibeventHandle::check_connect(const char* ip,const int port)
{
    
    for(auto it = bev_map.begin();it != bev_map.end();it++)
    {
        struct BevInfor & info = it->second;
        if(info.ip == ip && port == info.port)
        {
            return it->first;
        }
    }
    return -1;
}

int LibeventHandle::add_bufferevent_connect(const char* ip,const int port)
{
    struct sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);
    sin.sin_port =htons(port);

    struct bufferevent * bev  = bufferevent_socket_new(main_base,-1,BEV_OPT_CLOSE_ON_FREE );//| BEV_OPT_THREADSAFE);

    if(bev == NULL)
    {
        #if LIBEVENT_HANDLE_DEBUG
            int err = EVUTIL_SOCKET_ERROR();
            std::cout << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl; 
            std::cout << "bev address = "<< bev  << std::endl; 
        #endif // DEBUG0
        return -1;
    }

    int re;
    if ( re = bufferevent_socket_connect(bev,(struct sockaddr *)&sin,sizeof(sin)) < 0 )
    {
        #if LIBEVENT_HANDLE_DEBUG
            std::cout << "connect to "<< ip << ":"<< port << "failure !"<< std::endl; 
        #endif // DEBUG
        bufferevent_free(bev);
        return -1;
    }
    #if LIBEVENT_HANDLE_DEBUG
        std::cout << "bufferevent_socket_connect :"<< re << std::endl;
        std::cout << "bufferevent_getfd :"<< bufferevent_getfd(bev) << std::endl; 
        std::cout << "connect to "<< ip << ":"<< port << "success !"<< std::endl; 
    #endif // DEBUG

    bufferevent_setcb()
    struct BevInfor peer;
    peer.ip = ip;
    peer.port = port;
    peer.bev =  bev;
    peer.is_listen = false;

    int peer_id = max_bev_id;
    max_bev_id++;
    bev_map[peer_id] = peer;

    return peer_id;
}

int LibeventHandle::add_bufferevent_listen(const char* ip,const int port,int socket_fd)
{
    struct bufferevent * bev  = bufferevent_socket_new(main_base,socket_fd,BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);

    struct BevInfor peer;
    peer.ip = ip;
    peer.port = port;
    peer.bev =  bev;
    peer.is_listen = true;

    int peer_id = max_bev_id;
    max_bev_id++;
    bev_map[peer_id] = peer;

    return peer_id;
}

int LibeventHandle::remove_buffevent(int id)
{
    if(bev_map.find(id) == bev_map.end())
    {
        return -1;
    }

    BevInfor info = bev_map[id];
    bufferevent_free(info.bev);
    bev_map.erase(id);
    return id;
}


int LibeventHandle::try_connect(const char* ip,const int port)
{
  
    int connect_id = -1;
    connect_id = check_connect(ip,port);
    if( connect_id  >=0 )
    {
        return connect_id;
    }  

    connect_id =  add_bufferevent_connect(ip,port);
    
    return connect_id;
}

void LibeventHandle::start_event_base_loop()
{
   #if LIBEVENT_HANDLE_DEBUG
   event_base_dispatch(main_base);
   #else
   event_base_loop(main_base,EVLOOP_NO_EXIT_ON_EMPTY);
   #endif
}

void LibeventHandle::event_loop_run(LibeventHandle * lib)
{
     #if LIBEVENT_HANDLE_DEBUG
            std::cout << "event base loop start"<< std::endl; 
    #endif // DEBUG

    lib->start_event_base_loop();

     #if LIBEVENT_HANDLE_DEBUG
            std::cout << "event base loop stop"<< std::endl; 
    #endif // DEBUG
}

bool LibeventHandle::send(const char * ip,const int port,const char * send_bytes,int send_size)
{
    int connect_id = try_connect(ip,port);
    if( connect_id < 0)
    {
        #if LIBEVENT_HANDLE_DEBUG
            std::cout << "send error ---- connect failure"<< std::endl; 
        #endif // DEBUG
        return false;
    }

    struct BevInfor & info = bev_map[connect_id];

    if(bufferevent_write(info.bev,send_bytes,send_size)<0)
    {
        #if LIBEVENT_HANDLE_DEBUG
            std::cout << "bufferevent_write error --- "<< "[ERROR " << EVUTIL_SOCKET_ERROR() << "] : " << evutil_socket_error_to_string(err) << std::endl; 
        #endif // DEBUG
        return false;
    }

    return true;

}

bool LibeventHandle::wait_recive(const char * ip,const int port,char * recive_bytes,int recive_size)
{

}



/****************  friend function  **********************/

void connlistener_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * sock, int socklen, void * arg)
{
    LibeventHandle * handle_ptr = (LibeventHandle *)arg;
    
    struct sockaddr_in * addr = (struct sockaddr_in *)sock;

    #if LIBEVENT_HANDLE_DEBUG
        std::cout << "Recive Connect!" << std::endl; 
    #endif // DEBUG

    if (handle_ptr->check_connect(inet_ntoa(addr->sin_addr),ntohl(addr->sin_port)) >= 0)
    {
        #if LIBEVENT_HANDLE_DEBUG
            std::cout << "we has connect to ip : "<<inet_ntoa(addr->sin_addr) <<" and port : " << ntohl(addr->sin_port) << std::endl; 
        #endif // DEBUG
    }
    else
    {
        #if LIBEVENT_HANDLE_DEBUG
            std::cout << "first view for ip : "<<inet_ntoa(addr->sin_addr) <<" and port : " << ntohl(addr->sin_port) << std::endl; 
        #endif // DEBUG
        handle_ptr->add_bufferevent_listen(inet_ntoa(addr->sin_addr),addr->sin_port,fd);
    }
}

void listen_error_cb(struct evconnlistener *, void *)
{
    
        #if LIBEVENT_HANDLE_DEBUG
            int err = EVUTIL_SOCKET_ERROR();
            std::cout << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl; 
            std::cout << "Listener ERROR" << std::endl; 
        #endif // DEBUG0
}


void default_bufferevent_read_cb(struct bufferevent *bev, void *ctx)
{
    LibeventHandle * lib = (LibeventHandle * )ctx;
    
}

void default_bufferevent_write_cb(struct bufferevent *bev, void *ctx)
{
    #if LIBEVENT_HANDLE_DEBUG
        std::cout << "write into bufferevent"<< std::endl; 
    #endif // DEBUG
}

