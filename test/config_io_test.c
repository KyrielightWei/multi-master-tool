#include "../src/include/config_io_api.h"
#include<string.h>
#include<stdio.h>

int main(void)
{
    char str[100];
    initConfigFile_C_API("config_test");
    int i;
    char ch[2];
    ch[0] = 'A';
    ch[1] = '\0';
    for (i = 0; i < 10; i++)
    {
        memset(str,0,sizeof(char)*100);
        strcpy(str,"ConfigKey@");
        strcat(str,ch);
        insertConfigItem_C_API(str);
        ch[0] = ch[0] + 1;
    }

    readConfig_C_API(NULL);
    outputConfig_C_API("config_output");

    ch[0] = 'A';
    ch[1] = '\0';

    char res[100];

    for (i = 0; i < 10; i++)
    {
        memset(str,0,sizeof(char)*100);
        strcpy(str,"ConfigKey@");
        strcat(str,ch);
        getConfigVal_C_API(str,res);
        printf("%s : %s\n",str,res);
        ch[0] = ch[0] + 1;
    }
    return 0;
}