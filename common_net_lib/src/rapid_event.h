/**
 * inherit from NetworkHandle
 * use rapidjson and libevent
*/

#include"network_handle.h"
#include <iostream>
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

void conn_writecb(struct bufferevent *serbev, void *user_data)
    {
        struct evbuffer *output = bufferevent_get_output(serbev);
        if (evbuffer_get_length(output) == 0) {
            printf("send hello to client\n");
        }
    }

void conn_readcb(struct bufferevent *serbev, void *user_data)
    {
        struct evbuffer *input = bufferevent_get_input(serbev);
        int len=evbuffer_get_length(input);
        if (len == 0) {
            printf("no recive\n");
            bufferevent_free(serbev);
        }else{
            unsigned char * p=evbuffer_pullup(input,len);
            printf("data from client: %s\n",p);
            evbuffer_drain(input,len);
        }
    }

void client_readcb(struct bufferevent *clibev, void *user_data)
    {
        struct evbuffer *input = bufferevent_get_input(clibev);
        int len=evbuffer_get_length(input);
        if (len == 0) {
            printf("no recive\n");
            bufferevent_free(clibev);
        }else{
            unsigned char * p=evbuffer_pullup(input,len);
           // ssttr=evbuffer_pullup(input,len);
            printf("data from server: %s\n",p);
            evbuffer_drain(input,len);
        }
    }

// void conn_eventcb(struct bufferevent *bev, short events, void *arg)
// {
//     char* ip = (char*)arg;
//     if (events & BEV_EVENT_EOF)
//     {
//         printf("connection closed:%s",ip);
//     }
//     else if (events & BEV_EVENT_ERROR)
//     {
//         printf("some other error!");
//     }
 
//     bufferevent_free(bev);
//     bev = NULL;
//     printf("bufferevent free");
// }

  void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
    {
        struct event_base *base = (struct event_base*)user_data;
        struct bufferevent *sbev;

        sbev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        if (!sbev) {
            fprintf(stderr, "Error constructing bufferevent!");
            event_base_loopbreak(base);
            return;
        }
        bufferevent_setcb(sbev, conn_readcb,conn_writecb, NULL, NULL);
        bufferevent_enable(sbev, EV_WRITE);
        bufferevent_enable(sbev, EV_READ);

        bufferevent_write(sbev, "you are connecting to the service", 
             strlen("you are connecting to the service"));
    }




class LibeventHandle:public NetworkHandle
{
    //class_name = "RapidEventHandle";
private:
    std::string s;
    struct event_base *main_base;
    struct evconnlistener *conn_listener;
    struct sockaddr_in sin;
   //struct bufferevent *bev;
    bool isSever;
    struct bufferevent *client_bev;

public:
    
    bool init_listener()
    {
        isSever=true;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(9955);

        conn_listener = evconnlistener_new_bind(main_base, listener_cb, (void *)main_base, LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&sin, sizeof(sin));
        if (!conn_listener) {
            fprintf(stderr, "Could not create a listener!\n");
            return false;
        }

        event_base_dispatch(main_base);
        return true;
    }

    void init_handle()
    {
    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(0x0201, &wsa_data);
    #endif

        main_base=event_base_new();
        if (!main_base) {
            fprintf(stderr, "Could not initialize libevent!\n");
            return;
        }
        isSever=false;
    }

    void free_handle()
    {
        if(isSever)
        {
            evconnlistener_free(conn_listener);
        }
        event_base_free(main_base);
    }

   bool try_connect(const char* ip,const unsigned int port)
   {
        char *server_ip=(char *)ip;
        int server_port=port;
        int socketfd = socket(AF_INET,SOCK_STREAM,0);
        struct sockaddr_in sockaddr;
        memset(&sockaddr,0,sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(server_port);
        inet_pton(AF_INET,server_ip,&sockaddr.sin_addr);
        if((connect(socketfd,(struct sockaddr*)&sockaddr,sizeof(sockaddr))) < 0 )
            {
                printf("connect to server failure!\n");
                return false;
            }
        else
        {
            printf("connect to server successfully!\n");
        }
    
        client_bev = bufferevent_socket_new(main_base, socketfd, BEV_OPT_CLOSE_ON_FREE);
        if (!client_bev) {
            fprintf(stderr, "Error constructing bufferevent!");
            event_base_loopbreak(main_base);
            return false;
        }
        bufferevent_setcb(client_bev,client_readcb, NULL, NULL, NULL);
        bufferevent_enable(client_bev, EV_WRITE);
        bufferevent_enable(client_bev, EV_READ);
        return true;
    }

    void send(const char* str)
    {
        struct evbuffer *output=bufferevent_get_output(client_bev);
        int len=evbuffer_get_length(output);
        struct evbuffer *input = bufferevent_get_input(client_bev);
        int inlen=evbuffer_get_length(input);
        evbuffer_drain(output,len);
        evbuffer_drain(input,inlen);
        // if (len > 0) {
        //     evbuffer_drain(output,len);
        // }
        bufferevent_write(client_bev,str,strlen(str));
    }

    void start()
    {
        event_base_dispatch(main_base);
    }

    unsigned char * recive()
    {
        // printf("come here");
        // unsigned char * str=(unsigned char *)"lk";
        // struct evbuffer *input = bufferevent_get_input(client_bev);
        // int len=evbuffer_get_length(input);
        // unsigned char * p=evbuffer_pullup(input,len);
        // printf("data: %s\n",p);
        // return p;
        unsigned char * r=(unsigned char *)"1122";
        return r;
    }
};

    