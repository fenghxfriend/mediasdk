#ifndef __RS_SEMAPHORE_H__
#define __RS_SEMAPHORE_H__

#include <condition_variable>
#include <mutex>
#include <atomic>

class RSSemaphore
{
public:
    RSSemaphore(int val)
    :m_semAtomic(val)
    ,m_condition()
    ,m_mutex()
    {

    }

    ~RSSemaphore(void)
    {

    }

    void wait(void)
    {
        int val = --m_semAtomic;
        if (0 > val) {
            std::unique_lock<std::mutex> lck(m_mutex);
            m_condition.wait(lck);
            lck.unlock();
        }
    }

    bool waitForTime(int ms)
    {
        std::atomic<bool> res;
        res = true;
        if(0 > m_semAtomic) {
            std::unique_lock<std::mutex> lck(m_mutex);
            std::cv_status waitStatus = m_condition.wait_for(lck, std::chrono::milliseconds(ms));
            res = waitStatus != std::cv_status::timeout ? true : false;
            lck.unlock();
        }
        return res;
    }

    void post(void)
    {
        ++m_semAtomic;
        std::unique_lock<std::mutex> lck(m_mutex);
        m_condition.notify_all();
        lck.unlock();
    }

protected:
    std::atomic<int> m_semAtomic;
    std::condition_variable m_condition;
    std::mutex m_mutex;
};

#endif