# lock table 代码简述
## 文件列表
- client.h & client.cpp：客户端操作，包括
    1. 发送锁请求
    2. 客户端需要一个消息接收者，以接收等到锁的通知
- server.h &server.cpp：服务端操作，包括
    1. 维护锁表
    2. 服务端通知客户端等待的锁已拿到时，需要一个消息发送者
- main-client.cpp &main-server.cpp：测试实例
- lock_table.proto & lock_inform.proto:\
    客户端发送锁请求给服务端，服务端通知客户端锁已拿到这两个消息内容及处理的方法都是不同的，所以这里用了两个proto文件
- CMakeLists.txt：cmake文件，做了三处更改，加了Proto文件、log模块、以及多文件编译
    ```
    line 60:
    FILE(GLOB protofiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HEADERS ${protofiles})

    line 64
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    line 103:
    add_executable(client client.cpp main-client.cpp ${PROTO_HEADERS} ${PROTO_SRCS})
    ad`d_executable(server server.cpp main-server.cpp ${PROTO_HEADERS} ${PROTO_SRCS})
    ````

## 设计简述
- 服务端维护锁表，记录table-page目前的锁是什么类型（读锁、写锁、无锁），当前持锁的客户端集合，以及当前正在等待锁的客户端集合
- 客户端发起锁请求，请求有：表id，页id，锁类型（读 / 写 / 释放）
- 流程：
    1. 客户端发起锁请求，除了请求内容外，还会发送本客户端上用来接收停止阻塞消息的消息接收者的端口，以便之后服务端能知道如何通知客户端不用继续阻塞
    2. 服务端接收锁请求，可以给锁直接回复ok，若有锁冲突回复wait，客户端阻塞
    3. 当前某个table-page上原有的锁都被释放，则从等待队列中取客户端继续服务，并通知该客户端已经拿到锁
    4. 被阻塞的客户端收到拿到锁的请求后退出阻塞，继续工作
- 运行过程中会产生日志到server.log和client.log文件，日志中用端口号来标识此条日志来自于哪个客户端或服务端

## 测试
- 测试锁未拿到是否会阻塞\
    结果：锁未拿到客户端会进入阻塞状态，已有锁释放后原被阻塞的客户端会继续运行
- 多个客户端批量发送请求\
    结果：多个客户端只发送读请求（不会有阻塞的状态）：起16个客户端，每个客户端发送10000条读锁请求，服务端总计处理了160000条请求，没有丢失
