#include "remote_page_lock_manager.h"

ulint RemotePageLockManager::cal_fold(uint32_t m_space,uint32_t m_page_no)
{
    ulint m_fold = (m_space << 20) + m_space + m_page_no;
    return m_fold;
}