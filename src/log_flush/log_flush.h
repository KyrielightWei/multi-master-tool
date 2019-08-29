#ifndef LOG_FLUSH_CLASS
#define LOG_FLUSH_CLASS

#include<vector>
#include<string>
#include<fstream>
#include<pthread.h>
#include<ctime>
#include<cstring>

class LogFlush
{
private:
    std::string path;
    pthread_mutex_t mutex;
    std::ofstream output;
    char time_buffer[100];
    bool has_init;

    void getTime();
public:
    LogFlush(const LogFlush&){}
    LogFlush() {has_init = false;}
    void log_append(const char * val);
    void init(const char * zpath);
    ~LogFlush();
};


#endif // !LOG_FLUSH_CLASS