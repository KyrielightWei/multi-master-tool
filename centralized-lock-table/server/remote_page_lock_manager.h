#ifndef REMOTE_PAGE_LOCK_MANAGER_HEADER
#define REMOTE_PAGE_LOCK_MANAGER_HEADER

#include <list>
#include <map>

// typedef unsigned short uint16_t;
// typedef unsigned int uint32_t;
// typedef unsigned long long uint64_t;

typedef unsigned long int ulint;

/**
 * Version Update rule
 * 1. Version_no start from 0.It will increase but not reduce.
 * 2. When lock manager recive a write request about a page, it will add version_no of the page.
 * 3. Version_no is independent of any mysql node.
 * **/

struct PageLock_t
{
    ulint m_fold;
    uint32_t space_id;
    uint32_t page_no;
    uint64_t version_no;
    int node_id; //page location
    //std::map<int,uint64_t> history_version_map; // pair : node_id <-> verison_no  // client
};


class RemotePageLockManager
{
    public:
    void init();
    void free();

    static ulint cal_fold(uint32_t m_space,uint32_t m_page_no);

    private:
    std::map<ulint,PageLock_t *> page_lock_map;

};


#endif // !REMOTE_PAGE_LOCK_MANAGER.h