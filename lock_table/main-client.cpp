/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/main-client.cpp
 */
 
 #include "client.h"
#include "server.h"
#include <string>
#include <random>
#include <thread>
#include <time.h> 
#include <queue>
#include <utility>
#include <thread>

client_server c_s;
RemoteLockTable re;

void run_server_in_client()
{
    c_s.run();
}

void client_test()
{
    TableID table_id ;
    PageID page_id ;
    std::string request_type ;

    // while(std::cin>>table_id>>page_id>>request_type)
    for(int i=0;i<10000;i++)
    {   
        std::queue<std::pair<int,int>> pp;
        std::pair<int,int> pirtmp;
        int tt;std::string ss;
        table_id=rand() % 20;
        page_id=rand() % 10;
        // tt=rand() % 3;
        // if(tt==0) {
            request_type="read";
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

        re.set_message(table_id,page_id,request_type,c_s.get_port());
        re.send_request();
        re.get_response();
    }
}

int main(int argc, char ** argv)
{
    std::cout<<"cin port ";
    int portt;
    std::cin>>portt;
    c_s.set_port(portt);
    c_s.init();
    re.init();

    std::thread client_s_server(run_server_in_client);
    std::thread client_tes(client_test);
    client_s_server.detach();
    client_tes.detach();
    

    sleep(12000);

    return 0;
}