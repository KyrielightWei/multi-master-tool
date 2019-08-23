
#include"../src/include/performance.h"
#include<stddef.h>

int main() 
{
    createIndicatior_C_API("/tmp/performance_test","test_indicatior");
    int i = 0;
    for (i = 0; i < 3200; i++)
    {
        addPerformanceRecord_C_API("test_indicatior",i);
    }
    finishRecord_C_API(NULL);
   
    
    return 0;
}