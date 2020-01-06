#ifndef LOCK_COMMON_HEADER
#define LOCK_COMMON_HEADER

#include<vector>
#include<list>
#include "uthash.h"

typedef unsigned long int ulint;

enum PageLockType
{
    R_LOCK,
    W_LOCK,
    UNLOCK
};

enum PageReplyType
{
    LOCK_SUCCESS,
    LOCK_FAIL,
    UNLOCK_SUCCESS,
    UNLOCK_FAIL
};

/**
 * Version Update rule
 * 1. Version_no start from 0.It will increase but not reduce.
 * 2. When lock manager recive a write request about a page, it will add version_no of the page.
 * 3. Version_no is independent of any mysql node.
 *
 * Node will require a page from lock manager before requiring from storage.
 * **/

/* Lock Table Item Information */
struct PageLock_t
{
    ulint m_fold;  /* hash key */
    uint32_t space_id;
    uint32_t page_no;
    uint64_t version_no;
    PageLockType lock;
    const char * page_holder; //page location
    std::list<const char *> lock_holder;
    std::list<const char *> wait_list;
    UT_hash_handle hash_handle;
};

/* Lock Request
1. Write Page : send write lock request;
2. Read Page : if the page is in local buffer pool,mysql node will not send lock request and read local page; (Log sync mechanism will update page later)
*/
struct PageLockRequest
{
    uint32_t space_id;
    uint32_t page_no;
    uint64_t version_no; // if not execute writing operation on latest page
    PageLockType lock_type;
};

/* Lock reply*/
struct PageLockReply
{
    uint32_t space_id;
    uint32_t page_no;
    const char * page_holder;
    PageReplyType reply_type;
};


#endif
