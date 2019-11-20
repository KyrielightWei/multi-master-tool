#include "include/easylogger.h"
#include <iostream>

int main(void)
{
    // EasyLogger("test_log","./async_log.txt",EasyLogger::error) << "test log message";
    //   EasyLogger("test_log","./async_log.txt",EasyLogger::info) << "test log message";
    //    EasyLogger("test_log","./async_log.txt",EasyLogger::debug) << "test log message";
    EasyLogger("test_log","./async_log.txt",EasyLogger::warn) << "test log message";
    EasyLoggerWithTrace("./async_log.txt",EasyLogger::warn) << "test log message";
    EasyLoggerWithTrace("./async_log.txt",EasyLogger::error) << "test log message";
    return 0;
}