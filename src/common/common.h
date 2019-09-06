#ifndef COMMON_FUNCTION_HEADER
#define COMMON_FUNCTION_HEADER

#include <vector>
#include <string>
#include <iostream>
#include<assert.h>
#include <sys/time.h>


using namespace std;

vector<string> split(const string &s, const string &seperator);

string& trim(string &s) ;

long long getCurrentTime_ms();
long long getCurrentTime_us();

#endif // !COMMON_FUNCTION_HEADER