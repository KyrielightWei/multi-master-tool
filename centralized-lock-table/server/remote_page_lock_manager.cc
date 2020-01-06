#include "remote_page_lock_manager.h"
#ifdef REMOTE_PAGE_LOCK_MANAGER_DEBUG
#include <iostream>
#endif

/********************************
 * Static Functions
*********************************/
ulint RemotePageLockManager::cal_fold(uint32_t m_space,uint32_t m_page_no)
{
    ulint m_fold = (m_space << 20) + m_space + m_page_no;
    return m_fold;
}

void RemotePageLockManager::lock_request_callback(EventMessageHandle * mess_handle,EventMessage * mess,void * arg)
{
    #ifdef REMOTE_PAGE_LOCK_MANAGER_DEBUG
    std::cout << "Lock Request Callback invoked " <<std::endl;
    #endif
    RemotePageLockManager * lock_manager = (RemotePageLockManager * )arg;
    PageLockReply reply;
    lock_manager->handlePageLockRequest((struct PageLockRequest *)mess->message,&reply);

    EventMessage replyMsg;
    replyMsg.prepare_send(mess->group_name,"PageLockReply",mess->send_host_name, &reply,sizeof(PageLockReply));
}

/********************************
 * Public Member Functions
*********************************/
void RemotePageLockManager::init(const char * host_config_path,const char * mess_config_path)
{
    msg_handle.init_handle(host_config_path, mess_config_path);

    page_lock_hash = NULL; //init hash table

    /**
     * Register callbacks for lock requests
     *
     * todo:
     *      1. recive r lock request
     *      2. recive w lock request
     *      3. recive unlock request
    */
    msg_handle.register_recive_handler( "centralized_lock", "PageLockRequest",lock_request_callback,NULL);
}

void RemotePageLockManager::free()
{
    msg_handle.free_handle();
}

void RemotePageLockManager::listen()
{
    while(1)
    {
        // Keep Run
    }
}

int RemotePageLockManager::handlePageLockRequest(PageLockRequest * request,PageLockReply * reply)
{
    /*
    1. Check lock request, construct page lock reply
        Return ---- 0 : no page_lock_t
        Return ---- 1 : lock succeeded
        Return ---- -1 : lock failed
    2. if 0 ,insert page_lock_t
    */
}

/********************************
 * Private Member Functions
*********************************/

int RemotePageLockManager::check_lock_request(PageLockRequest * request,PageLockReply * reply)
{

}

int RemotePageLockManager::insert_page_lock_t(PageLockRequest * request)
{

}