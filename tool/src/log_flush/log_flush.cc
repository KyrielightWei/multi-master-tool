#include"log_flush.h"

#include"../include/log_api.h"

std::list<LogFlush> log_list;

/**
 * LogFlush
*/
void LogFlush::getTime()
{
    time_t t;
    struct tm *lt;
    time(&t);//获取Unix时间戳。
    lt = localtime(&t);//转为时间结构。

    sprintf(time_buffer, "%d/%d/%d %d:%d:%d   ", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果
}

void LogFlush::init(const char * zpath)
{
    path = zpath;
    pthread_mutex_init(&mutex,NULL);
    output.open(path,std::ios::app | std::ios::out);
    has_init = true;
}

LogFlush::~LogFlush()
{
    if(has_init)
    {
        output.close();
        pthread_mutex_destroy(&mutex);
    }
}

void LogFlush::log_append(const char * val)
{
    pthread_mutex_lock(&mutex);
    pthread_t thread_id = pthread_self();
    memset(time_buffer,0,sizeof(char)*100);
    getTime();
    output << time_buffer << "thread id：" << thread_id << "       " << val << std::endl;
    pthread_mutex_unlock(&mutex);
}

/*
 * API
 */
int log_init(const char * path)
{
    log_list.push_back(LogFlush());
    log_list.back().init(path);
    return log_list.size()-1;
}

void log_append(int log_num,const char * val)
{
    size_t  i = 0;
    std::list<LogFlush>::iterator it = log_list.begin();
    for (; i < log_num; i++)
    {
        it++;
    }
    it->log_append(val);
}

int log_init_C_API(const char * path)
{
    log_init(path);
}

void log_append_C_API(int log_num,const char * val)
{
    log_append(log_num,val);
}






