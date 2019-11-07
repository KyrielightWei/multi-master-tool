/***
 * Common Network transmission handle
 * Basic Class
*/
#include<string>

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
    virtual std::string get_packet_type(){return "Packet";};

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

    std::string get_packet_type(){return "SimplePacket";};

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
    virtual void init_handle()=0;
    virtual void free_handle()=0;

    /*connect*/
    virtual bool ready_connect()=0;
    virtual bool try_connect(const char * ip,const unsigned int port)=0;

    /*Remote IO*/
    virtual bool sendPacket(const Packet * send_packet)=0;
    virtual bool receivePacket(Packet * rec_packet)=0;

    /*serialization*/  //basic

    virtual std::string get_handle_type(){return "NetworkHandle";};
    
};

