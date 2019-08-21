#ifndef PERFOR_STATISTICS
#define PERFOR_STATISTICS

#include<pthread.h>
#include<string>
#include<list>
#include"../include/performance.h"

//============================================================
/***
 *  Indicatior
 * */
class PerformanceIndicatior
{
private:
    std::string name;
    std::list<VALUE_TYPE> record_list;

    std::list<CsvFile*> csv_list;

    int64_t count;

    unsigned int op_flag;

    void appendRecord(VALUE_TYPE val);
    PerformanceIndicatior();

public:
    static PerformanceIndicatior * generateIndicator(const char * name,uint flag);

    void addRecord(VALUE_TYPE val);
    //bool isRecordType();
   // bool isThis(const char * name);
    //bool finishRecord();
};

std::list<PerformanceIndicatior *> per_list;
typedef std::list<PerformanceIndicatior *>::iterator IndicatiorListIterator;

PerformanceIndicatior * searchIndicatior(const char * name);
void clearPerformanceData(const char * name);

//============================================================
/***
 *  Result File
 * */

class CsvFile
{
private:
    std::string path;
    std::list<PerformanceIndicatior *> col_list;

    uint64_t row_count;
    
#if FLUSH_MODE==THREAD_FLUSH
    uint64_t flush_index;
    bool finish;
    pthread_t csv_pthread;
    pthread_cond_t csv_cond;
    pthread_mutex_t csv_mutex; 

    void flushOrSleep();
    static void * flushThreadRun(void * file);
#endif 
   void flushCsv(uint64_t n);
public:

    CsvFile(const char * csvPath);  
    ~CsvFile();
    void addCol(PerformanceIndicatior * newCol);

   // void startCsvFlush();
};

class ResultFile
{
private:

public:
};

#endif // !PERFOR_STATISTICS