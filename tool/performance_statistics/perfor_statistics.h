#ifndef PERFOR_STATISTICS
#define PERFOR_STATISTICS

#include<pthread.h>
#include<string>
#include<list>
#include<iostream>
#include<fstream>
#include<cassert>
#include<cstring>
#include"../include/performance_api.h"

/**
 * We have two modes to flush statistics data into File:
 * 1. Thread Mode : when all Columns have 100 rows,wake up a flush thread, append 100 rows into csv File
 * 2. OneTime Mode: call a function to make all rows append into file
*/
#define THREAD_FLUSH 0
#define ONETIME_FLUSH 1

#define FLUSH_MODE ONETIME_FLUSH

#define THREAD_FLUSH_UNIT 1000

//============================================================
/***
 *  Indicatior
 * */
class PerformanceIndicatior
{
private:
    std::string name;
    std::string path;
    std::list<VALUE_TYPE> record_list;
    typedef std::list<VALUE_TYPE>::iterator RecordListIterator;
 
    int64_t count;
    uint64_t flush_index;

    RecordListIterator flush_start_iterator;
    
    PerformanceIndicatior(){}

    std::ofstream output_csv;

#if FLUSH_MODE==THREAD_FLUSH
    bool nowflush;
    bool unfinish;
    pthread_t csv_pthread;
    pthread_cond_t csv_cond;
    pthread_mutex_t csv_mutex; 

    void flushOrSleep();
    static void * flushThreadRun(void * file);
#endif 
   void flushCsv(uint64_t n);
   
   int performance_type;

   VALUE_TYPE record_buffer;

public:
    static PerformanceIndicatior * generateIndicator(const char * name,const char * dirPath,int type = PERFORMANCE_TIME);

    void flushNow();
    bool isThis(const char * name);
    long long beginTimeRecord();
    long long endTimeRecord();
    void addRecord(VALUE_TYPE val);
    void finishRecord();
};


typedef std::list<PerformanceIndicatior *>::iterator IndicatiorListIterator;

PerformanceIndicatior * searchIndicatior(const char * name);

#endif // !PERFOR_STATISTICS