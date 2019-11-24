/***
 * Common Network transmission handle
 * Basic Class
*/
#ifndef NETWORK_HANDLE_HEADER

#define NETWORK_HANDLE_HEADER

#include<string>
#include <iostream>

// int testINT();

/**
 * ToDo
 * NetWork基类需要的接口
 * 序列化类接口
*/

/**
 * Note
 * 继续使用构造函数与析构函数，虽然会提高代码的复杂性，但提供了使用用户指定方式分配空间的可能
 * 为了简化代码，构造函数与析构函数处理尽可能少的工作，初始化与销毁工作交给init与free函数处理
*/
class Packet
{
    public:
    virtual unsigned int get_packet_size()=0;

    virtual void * get_val_ptr()=0;
};


/* simple type packet*/
template<typename T>
class SimplePacket:public Packet
{
  //  class_name = "SimplePacket";
    
    public:
    
    void * get_val_ptr() override;
    unsigned int get_packet_size() override;
    void set_val(const T & val);

    SimplePacket(const T & val)
    {
        value = val;
    }

    private:
    T value;
};


class NetworkHandle
{
    public:
    virtual bool init_handle()=0;
    //init_clientHandle
    virtual bool free_handle()=0;
   
    /*connect*///client端任务，server端只用监听是否有连接到来就行
    //virtual bool ready_connect()=0;
    // virtual bool try_connect(const char* ip,const unsigned int port)=0;
    // virtual bool listen_connect()=0;

    /*Remote IO*/
    //virtual bool sendPacket(const Packet * send_packet)=0;
    //virtual bool receivePacket(Packet * rec_packet)=0;
    virtual bool send(const char * ip,const int port,const char * send_bytes,int send_size)=0;
    virtual bool wait_recive(const char * ip,const int port,char * recive_bytes,int recive_size)=0;
    //virtual bool set_recive_callback()=0;
    //virtual bool send_wait_reply(const char * ip,const int port,const char * send_packet,char * recive_packet)=0;
    /*serialization*/  //basic
};


/*************************************************************
 *      Class SimplePacket<T> implementation
 * ***********************************************************
*/
template<typename T>
void * SimplePacket<T>::get_val_ptr()
{
    return (void *)&value;
}

template<typename T>
void SimplePacket<T>::set_val(const T & val)
{
    value = val;
}

template<typename T>
unsigned int SimplePacket<T>::get_packet_size()
{
    return sizeof(T);
}

#endif // !NETWORK_HANDLE_HEADER