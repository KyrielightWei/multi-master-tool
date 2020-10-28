/*
 * @Author: liu
 * @Date: 2020-10-22 10:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/main-server.cpp
 */
 
#include "server.h"

int main()
{
    LockTableServer se;

    se.init();  
    se.run();

    return 0;
}