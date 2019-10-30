
#include"../src/include/performance_api.h"
#include<stddef.h>
#include<stdio.h>

int main() 
{
    createIndicatior_C_API(".","test_indicatior");
    int i = 0;
    for (i = 0; i < 3200; i++)
    {
        addPerformanceRecord_C_API("test_indicatior",i);
    }
    
    createIndicatior_C_API(".","time_indicatior");
    int ti,j;
    long long temp = 0;
    for(ti =0 ; ti < 20; ti++)
    {

        temp = beginIndicatiorTimeRecord_C_API("time_indicatior");
        printf ("begin: %ld  ",temp);
        for (j = 0; j < 1000000000; j++)
        {
        }
        temp = endIndicatiorTimeRecord_C_API("time_indicatior");
        printf("end: %ld  \n",temp);
    }

    //flushNow_C_API(NULL);
   finishRecord_C_API(NULL);
   
//    while (1)
//    {
//        /* code */
//    }
   
    return 0;
}