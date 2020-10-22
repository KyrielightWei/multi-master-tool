# lock table 代码简述
## 文件列表
- client.h & client.cpp：client相关操作，包括
    1. 发送锁请求的客户端本身
    2. 服务端通知客户端锁已拿到时需要的client
    > 即客户端和服务端的两个client
- server.h &server.cpp：server相关操作，包括
    1. 维护锁表的服务端本身
    2. 客户端需要起一个server，以接收等到锁的通知
    > 即客户端和服务端的两个server
- main-client.cpp &main-server.cpp：主函数
- lock_table.proto & STOC.proto:\
    客户端发送锁请求给服务端，服务端通知客户端锁已拿到这两个消息内容及处理的方法都是不同的，所以这里用了两个proto文件
- CMakeLists.txt：cmake文件，做了两处更改，加了Proto文件以及多文件编译
    ```
    line 60:
    FILE(GLOB protofiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HEADERS ${protofiles})

    line 103:
    add_executable(client client.cpp server.cpp main-client.cpp ${PROTO_HEADERS} ${PROTO_SRCS})
    ad`d_executable(server server.cpp client.cpp main-server.cpp ${PROTO_HEADERS} ${PROTO_SRCS})
    ````

## 设计简述
- 服务端维护锁表，记录table-page目前的锁是什么类型（读锁、写锁、无锁），当前持锁的客户端集合，以及当前正在等待锁的客户端集合
- 客户端发起锁请求，请求有：表id，页id，锁类型（读、写、释放）
- 流程：
    1. 客户端发起锁请求，除了请求内容外，还会发送本客户端上用来接收停止阻塞的消息的server的端口，以便之后服务端能知道如何通知客户端不用继续阻塞
    2. 服务端接收锁请求，可以给锁直接回复ok，若有锁冲突回复wait，客户端阻塞
    3. 当前某个table-page上原有的锁都被释放，则从等待队列中取客户端继续服务，并通知该客户端已经拿到锁
    4. 被阻塞的客户端收到拿到锁的请求后退出阻塞，继续工作

## 测试
- 测试锁未拿到是否会阻塞\
结果：锁未拿到客户端会进入阻塞状态，已有锁释放后原被阻塞的客户端会继续运行
- 多个客户端批量发送请求
    - 起了三个客户端发送请求，但是会很快有锁阻塞，也没释放掉（随机的命令很难释放掉那个阻塞的锁）
    - 多个客户端只发送读请求（不会有阻塞的状态）：起16个客户端，每个客户端发送10000条读锁请求，服务端总计处理了160000条请求,，没有丢失
