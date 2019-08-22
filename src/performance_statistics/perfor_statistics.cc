
#include "perfor_statistics.h"

/**
 * class PerformanceIndicatior
*/
PerformanceIndicatior * PerformanceIndicatior::generateIndicator(const char * name,const char * dirPath)
{
    PerformanceIndicatior * newPerformance = new PerformanceIndicatior;
    newPerformance->name = name;
    newPerformance->path = dirPath;
    newPerformance->path = newPerformance->path + "/" + newPerformance->name + ".csv";
    newPerformance->record_list.clear;
    newPerformance->count = 0;
    newPerformance->flush_index = 0;

#if FLUSH_MODE==THREAD_FLUSH
    pthread_cond_init(&newPerformance->csv_cond,NULL);
    pthread_mutex_init(&newPerformance->csv_mutex,NULL);
    newPerformance->finish = false;
    pthread_create(&newPerformance->csv_pthread,NULL,flushThreadRun,newPerformance);
#endif

    return newPerformance;
}


#if FLUSH_MODE==THREAD_FLUSH
void PerformanceIndicatior::flushOrSleep()
{
    pthread_mutex_lock(&csv_mutex);
    while (count - flush_index < THREAD_FLUSH_UNIT && finish)
    {
        pthread_cond_wait(&csv_cond,&csv_mutex);
    }
    pthread_mutex_unlock(&csv_mutex);
}

void * PerformanceIndicatior::flushThreadRun(void * file)
{
    PerformanceIndicatior * performanceI = (PerformanceIndicatior*)file;
    while (1)
    {
        performanceI->flushOrSleep();
        performanceI->flushCsv(THREAD_FLUSH_UNIT);
        performanceI->flush_index = performanceI->flush_index + THREAD_FLUSH_UNIT;
    }
}
#endif

void PerformanceIndicatior::flushCsv(uint64_t n)
{
    output_csv.open(path,std::ios::out | std::ios::app);
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
    assert(flush_start_iterator != record_list.end());
   
    for(uint64_t i =0; i < flushCount;i++)
    {
        output_csv << *flush_start_iterator << std::endl;
        if(i != flushCount -1)
            flush_start_iterator++;
    }
    output_csv.close();
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
    flushCsv(0);
}


/**
 * private function
 */
PerformanceIndicatior * searchIndicatior(const char * name)
{
    for (IndicatiorListIterator i = per_list.begin(); i != per_list.end(); i++)
    {
        if(i->isThis(name))
        {
            return *i;
        }
    }
    return NULL;
}

/**
 * API
*/

void clearPerformanceData(const char * name)
{
    if(name = NULL)
    {
        for (IndicatiorListIterator i = per_list.begin(); i != per_list.end(); i++)
        {
            delete *i;
        }
        per_list.clear();
    }
    else
    {
        PerformanceIndicatior * indicatior = searchIndicatior(name);
        delete indicatior;
        per_list.remove(indicatior);
    }
}



void createIndicatior(const char * dirPath,const char * name)
{
    PerformanceIndicatior * newIndicatior = NULL;
    newIndicatior = PerformanceIndicatior::generateIndicator(name,dirPath);
    per_list.push_back(newIndicatior);
}


void addPerformanceRecord(const char * name,VALUE_TYPE val)
{
    PerformanceIndicatior * current = NULL;
    current = searchIndicatior(name);
    current->addRecord(val);
}

