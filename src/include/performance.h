
#ifndef PERFORMANCE_INCLUDE
#define PERFORMANCE_INCLUDE

/**
 *  statistics type : how to handle statistics data
*/
#define STA_RECORD 0x1

/**
 * Type of performance indicatior value 
*/
#define VALUE_TYPE int64_t

/**
 * We have two modes to flush statistics data into File:
 * 1. Thread Mode : when all Columns have 100 rows,wake up a flush thread, append 100 rows into csv File
 * 2. OneTime Mode: call a function to make all rows append into file
*/
#define THREAD_FLUSH 0
#define ONETIME_FLUSH 1

#define FLUSH_MODE THREAD_FLUSH

#define THREAD_FLUSH_UNIT 1000

//============================================================
/***
 *  Performance API
 * */
//#define PERFORMANCE_C_API
#ifdef PERFORMANCE_C_API
extern “C” {
#endif // PERFORMANCE_C_API

void createIndicatior(const char * name,unsigned int flag);
void addPerformanceRecord(const char * name,VALUE_TYPE val);

#ifdef PERFORMANCE_C_API
}
#endif // PERFORMANCE_C_API

//============================================================

#endif // PERFORMANCE_INCLUDE
