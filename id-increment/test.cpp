
#include <iostream>
#include <pthread.h>
#include <unistd.h>
 
using namespace std; 
#define NUM_THREADS 64

void* run_client(void* args)
{
    system("/home/liuwenxin/multi-master-tool/id-increment/build/client --interval_ms=100");
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