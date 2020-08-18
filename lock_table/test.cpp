/*
 * @Author: liu
 * @Date: 2020-07-28 18:00
 * @LastEditors: Do not edit
 * @LastEditTime: 
 * @Description: file content
 * @FilePath: /multi-master-tool/lock_table/test.cpp
 */
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <map>
#include <set>
#include <stdlib.h>
#include <time.h> 
#include <string>
using namespace std; 
#define NUM_THREADS 64

void* run_client(void* args)
{
    system("./build/client --interval_ms=0");
    return 0;
}
 
int main()
{
    pthread_t tids[NUM_THREADS];
    for(int i = 0; i < NUM_THREADS; ++i)
    {
        int ret = pthread_create(&tids[i], NULL, run_client, NULL);
        if (ret != 0)
        {
           cout << "pthread_create error: error_code=" << ret << endl;
        }
    }
    pthread_exit(NULL);
}
