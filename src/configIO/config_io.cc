#include "../include/config_io_api.h"
#include "config_io.h"
#include "../common/common.cc"

#include<string>
#include<fstream>


/**
 * Class ConfigItem
*/
void ConfigFile::init(const char * zpath,const char * separ)
{
    path = zpath;
    items.clear();
    item_read_counts.clear();
    separator = separ;
    readCount = 0;
}

void ConfigFile::insertItem(const char * key)
{
    ConfigItem newItem;
    newItem.key = key;
    newItem.val = "";
    items.push_back(newItem);
    item_read_counts.push_back(0);
}

void ConfigFile::readConfig(const char * key)
{
    std::ifstream in;
    std::vector<std::string> result; 
    std::string line_key,line_val; 

    in.open(path, std::ios::in);
    if(!in) return;
    std::string line_buffer;
    while (!in.eof())
    {
        getline(in,line_buffer);
        result.clear();
        line_key.clear();
        line_val.clear();
    
        result = split(line_buffer,separator);

        if(result.size() >= 2)
        {
            line_key = result[0];   
            line_val = result[1];
        
            trim(line_key);
            trim(line_val);
 
            int index = findConfigIndex(line_key.c_str());
            if(index >= 0)
            {
                items[index].val = line_val;
                item_read_counts[index]++;
            }
            if(key != NULL && key == line_key)
            {
                in.close();
                return;
            }
        // std::cout << "********Read config*************" << std::endl;
        // std::cout << line_buffer << std::endl;
        // std::cout << line_key << std::endl;
        // std::cout << line_val << std::endl;
        // std::cout << index << std::endl;
        // std::cout << "********************************" << std::endl;
        }    
    }
    // std::cout << "while finish "  << std::endl;
    in.close();
}

void ConfigFile::getVal(const char * key,char * val)
{
    int index = findConfigIndex(key);
    //std::cout << key << " " << index << std::endl;
    assert(index >= 0);
    strcpy(val, items[index].val.c_str());
}

int ConfigFile::findConfigIndex(const char * key)
{
    int count = 0;
    ConfigIterator it = items.begin();
    for( ; it != items.end();it++)
    {
        if((*it).key == key)
        {
            return count;
        }
        count++;
    }
    return -1;
}

void ConfigFile::outputConfig(const char * zpath)
{
    std::ofstream out;
    out.open(zpath,std::ios::out);
    ConfigIterator it = items.begin();
    int i = 0;
    for (; it != items.end() ; it++)
    {
        out << "Config "<< i <<" : "<< items[i].key << " --- " << items[i].val << std::endl; 
        i++;
    }
    out.close();
}

/**
 * Config API
*/
ConfigFile config_file;

void initConfigFile(const char * zpath)
{
    config_file.init(zpath);
}


void initConfigFile(const char * zpath,const char * separator)
{
    config_file.init(zpath,separator);
}


void insertConfigItem(const char * key)
{
    config_file.insertItem(key);
}

void readConfig(const char * key)
{
    config_file.readConfig(key);
}

void getConfigVal(const char * key,char * val)
{
    config_file.getVal(key,val);
}

void outputConfig(const char * zpath)
{
    config_file.outputConfig(zpath);
}

void initConfigFile_C_API(const char * zpath)
{
    initConfigFile(zpath);
}

void initConfigFile_C_API_SEP(const char * zpath,const char * separator)
{
    initConfigFile(zpath,separator);
}

void insertConfigItem_C_API(const char * key)
{
    insertConfigItem(key);
}

void readConfig_C_API(const char * key)
{
    readConfig(key);
}

void getConfigVal_C_API(const char * key,char * val)
{
    getConfigVal(key,val);
}

void outputConfig_C_API(const char * zpath)
{
    outputConfig(zpath);
}