#ifndef REMOTE_PAGE_LOCK_MANAGER_HEADER
#define REMOTE_PAGE_LOCK_MANAGER_HEADER

#include <list>
#include <vector>
#include "event_mess_handle.h"

#include "lock_common.h"

#define REMOTE_PAGE_LOCK_MANAGER_DEBUG

class RemotePageLockManager
{
    public:
    static ulint cal_fold(uint32_t m_space,uint32_t m_page_no);

    static void lock_request_callback(EventMessageHandle * mess_handle,EventMessage * mess,void * arg);

    // public API
    void init(const char * host_config_path,const char * mess_config_path);
    void free();
    void listen(); // listen lock request

    //callback API
    int handlePageLockRequest(PageLockRequest * request,PageLockReply * reply);

    private:
    int check_lock_request(PageLockRequest * request,PageLockReply * reply);
    int insert_page_lock_t(PageLockRequest * request);

    struct PageLock_t * page_lock_hash;
    EventMessageHandle msg_handle;
};


#endif // !REMOTE_PAGE_LOCK_MANAGER.h