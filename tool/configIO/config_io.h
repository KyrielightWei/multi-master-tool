#ifndef CONFIG_IO_CLASS_HEADER
#define CONFIG_IO_CLASS_HEADER

#include<string>
#include<fstream>
#include<vector>
#include<assert.h>
#include<cstring>
#include<iostream>

struct ConfigItem 
{
    std::string key;
    std::string val;
};

class ConfigFile
{
private:
    std::string path;
    std::vector<ConfigItem> items;
    std::vector<int> item_read_counts;
    int readCount;
    std::string separator;
    int findConfigIndex(const char * key);
public:
    void init(const char * zpath,const char * separ = ":");
    void insertItem(const char * key);
    void readConfig(const char * key);  //NULL == all
    void getVal(const char * key,char * val);
    void outputConfig(const char * zpath);
};

typedef std::vector<ConfigItem>::iterator ConfigIterator; 

#endif // !CONFIG_IO_CLASS_HEADER