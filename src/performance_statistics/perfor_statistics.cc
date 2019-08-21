
#include "perfor_statistics.h"

/**
 * class PerformanceIndicatior
*/
PerformanceIndicatior * PerformanceIndicatior::generateIndicator(const char * name,uint flag)
{
    PerformanceIndicatior * newPerformance = new PerformanceIndicatior;
    newPerformance->name = name;
    newPerformance->record_list.clear;
    newPerformance->count = 0;
    return newPerformance;
}

/**
 * class CsvFile
*/
CsvFile::CsvFile(const char * csvPath)
{
    path  = csvPath;
    col_list.clear();
    row_count = 0;

#if FLUSH_MODE==THREAD_FLUSH
    flush_index = 0;
    pthread_cond_init(&csv_cond,NULL);
    pthread_mutex_init(&csv_mutex,NULL);
    finish = false;

    pthread_create(&csv_pthread,NULL,flushThreadRun,this);
#endif
}

#if FLUSH_MODE==THREAD_FLUSH
void CsvFile::flushOrSleep()
{
    pthread_mutex_lock(&csv_mutex);
    while (row_count - flush_index < THREAD_FLUSH_UNIT && finish)
    {
        pthread_cond_wait(&csv_cond,&csv_mutex);
    }
    pthread_mutex_unlock(&csv_mutex);
}

void * CsvFile::flushThreadRun(void * file)
{
    CsvFile * csv = (CsvFile*)file;
    while (1)
    {
        csv->flushOrSleep();
        csv->flushCsv(THREAD_FLUSH_UNIT);
        csv->flush_index = csv->flush_index + THREAD_FLUSH_UNIT;
    }
}
#endif

void CsvFile::flushCsv(uint64_t n)
{
    if(n == 0)
    {

    }
    else
    {

    }
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



/**
 * API
*/
void createIndicatior(const char * name,uint flag)
{
    PerformanceIndicatior * newIndicatior = NULL;
    newIndicatior = PerformanceIndicatior::generateIndicator(name,flag);
    per_list.push_back(newIndicatior);
}


void addPerformanceRecord(const char * name,VALUE_TYPE val)
{
    PerformanceIndicatior * current = NULL;
    current = searchIndicatior(name);
    current->addRecord(val);
}

