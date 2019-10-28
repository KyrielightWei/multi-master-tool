
#include "perfor_statistics.h"
#include<iostream>
#include "../common/common.h"

std::list<PerformanceIndicatior *> per_list;

//#define DEBUG 

/*
 * class PerformanceIndicatior
*/
PerformanceIndicatior * PerformanceIndicatior::generateIndicator(const char * name,const char * dirPath,int type)
{
    PerformanceIndicatior * newPerformance = new PerformanceIndicatior;
    newPerformance->name = name;
    newPerformance->path = dirPath;
    newPerformance->path = newPerformance->path + "/" + newPerformance->name + ".csv";
    
    #ifdef DEBUG
    std::cout << newPerformance->path << std::endl;
    #endif // 0
   
    newPerformance->record_list.clear();
    newPerformance->count = 0;
    newPerformance->flush_index = 0;

    newPerformance->performance_type = type;
    newPerformance->record_buffer = 0;

#if FLUSH_MODE==THREAD_FLUSH
    pthread_cond_init(&newPerformance->csv_cond,NULL);
    pthread_mutex_init(&newPerformance->csv_mutex,NULL);
    newPerformance->unfinish = true;
    newPerformance->nowflush = false;
    pthread_create(&newPerformance->csv_pthread,NULL,flushThreadRun,newPerformance);
#endif
    return newPerformance;
}


#if FLUSH_MODE==THREAD_FLUSH
void PerformanceIndicatior::flushOrSleep()
{
    
    pthread_mutex_lock(&csv_mutex);
    while (count - flush_index < THREAD_FLUSH_UNIT && unfinish && !nowflush)
    {
        pthread_cond_wait(&csv_cond,&csv_mutex);
    }
    pthread_mutex_unlock(&csv_mutex);
    #ifdef DEBUG
    std::cout <<std::endl<< "=======flushOrSleep=========" << std::endl;
    std::cout << "count - flush_index = "<< count - flush_index << std::endl;
    std::cout << "count = "<< count << std::endl;
    std::cout << "flush_index = "<< flush_index << std::endl;
    std::cout << "=======end of flushOrSleep=========" << std::endl << std::endl;
    #endif
}

void * PerformanceIndicatior::flushThreadRun(void * file)
{
    PerformanceIndicatior * performanceI = (PerformanceIndicatior*)file;
    while (1)
    {
        performanceI->flushOrSleep();
        if(performanceI->nowflush)
        {
#ifdef DEBUG
    std::cout << performanceI->name <<"######" << performanceI->flush_index << std::endl;
#endif // DEBUG
            performanceI->flushCsv(0);
            performanceI->nowflush = false;
            continue; 
        }
        if(performanceI->unfinish)
        {
            performanceI->flushCsv(THREAD_FLUSH_UNIT);
            //performanceI->flush_index = performanceI->flush_index + THREAD_FLUSH_UNIT;
        }
        else
        {
            performanceI->flushCsv(0);
            pthread_exit(NULL);
        }
       
    }
}
#endif

void PerformanceIndicatior::flushCsv(uint64_t n)
{
    output_csv.open(path.c_str(),std::ios::app | std::ios::out);
    #ifdef DEBUG
    std::cout <<std::endl<< "=======flushCsv=========" << std::endl;
    std::cout << path.c_str() << std::endl;
    std::cout << !output_csv<< std::endl;
    std::cout << "flush rows : "<<n<< std::endl;
    std::cout << "=======end of flushCsv=========" << std::endl << std::endl;
    #endif // 0
    uint64_t flushCount  = 0 ;
    if(n == 0 || n > count - flush_index)
    {
        flushCount = count - flush_index;
    }
    else
    {
        flushCount = n;
    }

     if(flush_index == 0)
    {
        flush_start_iterator = record_list.begin();
        output_csv << name <<  std::endl;
    }
    else
    {
        flush_start_iterator++;
    }
    //std::string message = name + " -- Performance Record has already finish";
    assert(record_list.empty()  || flush_start_iterator != record_list.end());
   
    for(uint64_t i =0; i < flushCount;i++)
    {
        output_csv << *flush_start_iterator << std::endl;
        if(i != flushCount -1)
            flush_start_iterator++;
    }
    output_csv.close();
    flush_index = flush_index + flushCount;
}

void PerformanceIndicatior::addRecord(VALUE_TYPE val)
{
    record_list.push_back(val);
    // if(count == 0)
    // {
    //     flush_start_iterator=record_list.begin();
    // }
    count = count + 1;

#if FLUSH_MODE==THREAD_FLUSH
   
    if(count - flush_index >= THREAD_FLUSH_UNIT)
    {
        pthread_cond_signal(&csv_cond);
    }
#endif
}

void PerformanceIndicatior::finishRecord()
{
    
#if FLUSH_MODE==THREAD_FLUSH
    pthread_mutex_lock(&csv_mutex);
    unfinish = false;
    pthread_mutex_unlock(&csv_mutex);
    pthread_cond_signal(&csv_cond);
    pthread_join(csv_pthread,NULL);
    pthread_mutex_destroy(&csv_mutex);
    pthread_cond_destroy(&csv_cond);
#else
    flushCsv(0);
#endif    
    record_list.clear();
    count = 0;
    flush_index = 0;
    flush_start_iterator = record_list.end();
}

void PerformanceIndicatior::flushNow()
{
#ifdef DEBUG
    std::cout << name <<"------" << record_list.size()<< std::endl;
#endif // DEBUG
#if FLUSH_MODE==THREAD_FLUSH
    pthread_mutex_lock(&csv_mutex);
    nowflush = true;
    pthread_mutex_unlock(&csv_mutex);
    pthread_cond_signal(&csv_cond);
#else
    flushCsv(0);
#endif    
}

bool PerformanceIndicatior::isThis(const char * name)
{
    return strcmp(this->name.c_str(),name)==0;
}

long long PerformanceIndicatior::beginTimeRecord()
{
    long long t = getCurrentTime_us();
    record_buffer = t;
    return t;
}

long long PerformanceIndicatior::endTimeRecord()
{
    long long t = getCurrentTime_us();
    record_buffer = t - record_buffer;
    addRecord(record_buffer);
    record_buffer = 0;
    return t;
}

/*
 * private function
 */
PerformanceIndicatior * searchIndicatior(const char * name)
{
    for (IndicatiorListIterator i = per_list.begin(); i != per_list.end(); i++)
    {
        if((*i)->isThis(name))
        {
            return *i;
        }
    }
    return NULL;
}



/*
 * API
*/



void createIndicatior(const char * dirPath,const char * name)
{
    PerformanceIndicatior * newIndicatior = NULL;
    newIndicatior = PerformanceIndicatior::generateIndicator(name,dirPath);
    per_list.push_back(newIndicatior);
}

long long beginIndicatiorTimeRecord(const char * name)
{
    PerformanceIndicatior * current = NULL;
    current = searchIndicatior(name);
    current->beginTimeRecord();
}

long long endIndicatiorTimeRecord(const char * name)
{
    PerformanceIndicatior * current = NULL;
    current = searchIndicatior(name);
    current->endTimeRecord();
}

void addPerformanceRecord(const char * name,VALUE_TYPE val)
{
    PerformanceIndicatior * current = NULL;
    current = searchIndicatior(name);
    current->addRecord(val);
}

void finishRecord(const char * name)
{
     if(name == NULL)
    {
        for (IndicatiorListIterator i = per_list.begin(); i != per_list.end(); i++)
        {
            (*i)->finishRecord();
            delete *i;
        }
        per_list.clear();
    }
    else
    {
        PerformanceIndicatior * indicatior = searchIndicatior(name);
        indicatior->finishRecord();
        delete indicatior;
        per_list.remove(indicatior);
    }
}

void flushNow(const char * name)
{
    if(name == NULL)
    {
        for (IndicatiorListIterator i = per_list.begin(); i != per_list.end(); i++)
        {
            (*i)->flushNow();
        }
    }
    else
    {
        PerformanceIndicatior * indicatior = searchIndicatior(name);
        indicatior->flushNow();
    }
}

/*
 * C_API
*/

void createIndicatior_C_API(const char * dirPath,const char * name)
{
    createIndicatior(dirPath,name);
}

void addPerformanceRecord_C_API(const char * name,VALUE_TYPE val)
{
    addPerformanceRecord(name,val);
}


void finishRecord_C_API(const char * name)
{
    finishRecord(name);
}

long long beginIndicatiorTimeRecord_C_API(const char * name)
{
    beginIndicatiorTimeRecord(name);
}

long long endIndicatiorTimeRecord_C_API(const char * name)
{
    endIndicatiorTimeRecord(name);
}

void flushNow_C_API(const char * name)
{
    flushNow(name);
}