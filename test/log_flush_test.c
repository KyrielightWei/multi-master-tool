
#include"../src/include/log_api.h"

#include <pthread.h>
#include <stdio.h>

char thread_logbuffer[1024];



void * func(void* arg)
{
    size_t i;
    for ( i= 1000; i < 2000; i++)
    {
        sprintf(thread_logbuffer,"log_val %d",i);
        log_append_C_API(0,thread_logbuffer);
    }
    return NULL;
}


int main(void)
{
    char logbuffer[1024];

    pthread_t pth;

    int log_num = log_init_C_API("/tmp/log_test/single_log");

    pthread_create(&pth,NULL,func,NULL);

    size_t i;
    for ( i= 0; i < 1000; i++)
    {
        sprintf(logbuffer,"log_val %d",i);
        printf("%s \n",logbuffer);
        log_append_C_API(log_num,logbuffer);
    }
    
    return 0;
}