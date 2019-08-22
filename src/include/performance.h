
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

//============================================================
/***
 *  Performance API
 * */
//#define PERFORMANCE_C_API
#ifdef PERFORMANCE_C_API
extern “C” {
#endif // PERFORMANCE_C_API

void createIndicatior(const char * dirPath,const char * name);
void addPerformanceRecord(const char * name,VALUE_TYPE val);
void clearPerformanceData(const char * name);
void finishRecord();

#ifdef PERFORMANCE_C_API
}
#endif // PERFORMANCE_C_API

//============================================================

#endif // PERFORMANCE_INCLUDE
