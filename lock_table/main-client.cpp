/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/main-client.cpp
 */

#include "client.h"
//test
#include <queue>
#include <utility>

InformReceiver inform_receiver;
RemoteLockTable request_sender;

void inform_receive()
{
    inform_receiver.run();
}

void client_instance()
{
    TableID table_id ;
    PageID page_id ;
    std::string request_type ;

    while(std::cin>>table_id>>page_id>>request_type)
    // for(int i=0;i<10000;i++)
    {   
        std::queue<std::pair<int,int>> pp;
        std::pair<int,int> pirtmp;
        // int tt;std::string ss;
        // table_id=rand() % 20;
        // page_id=rand() % 10;
        // // tt=rand() % 3;
        // // if(tt==0) {
        //     request_type="read";
        //     pirtmp=std::make_pair(table_id,page_id);
        //     pp.push(pirtmp);
        // }
        // else if(tt=1){
        //     request_type="write";
        //     pirtmp=std::make_pair(table_id,page_id);
        //     pp.push(pirtmp);
        // }
        // else{
        //     request_type="realse";
        //     table_id=pp.front().first;
        //     page_id=pp.front().second;
        //     pp.pop();
        // }

        request_sender.set_message(table_id,page_id,request_type,inform_receiver.get_port());
        request_sender.send_request();
        request_sender.get_response();
    }
}

int main(int argc, char ** argv)
{
    std::cout<<"cin port ";
    int portt;
    std::cin>>portt;
    inform_receiver.set_port(portt);
    inform_receiver.init();
    
    request_sender.init();

    std::thread receiver(inform_receive);
    std::thread client(client_instance);
    receiver.detach();
    client.detach();
    

    sleep(12000);

    return 0;
}