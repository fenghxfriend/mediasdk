/*******************************************************************************
 *        Module: common
 *          File: 
 * Functionality: auto locker
 *       Related: 
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 PAOMIANTV, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_AUTOLOCK_H_
#define _PAOMIANTV_AUTOLOCK_H_

#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include "autolog.h"

namespace paomiantv {

#define BEGIN_AUTOLOCK_TRACE(pLock) \
    do                              \
    {                               \
        CAutoLockTrace cAutoLock((pLock), __FILE__, __FUNCTION__, __LINE__);

#define BEGIN_AUTOLOCK(pLock) { CAutoLock cAutoLock((pLock))

#define END_AUTOLOCK }


    class ISemaphore {
    public:
        ISemaphore() {};

        virtual ~ISemaphore() {}

        virtual void wait() = 0;

        virtual void post() = 0;

    };

    class CSemaphore : public ISemaphore {
    public:
        CSemaphore() : ISemaphore() {
            CSemaphore(0);
        }

        CSemaphore(u32 uCount) {
            s32 re = sem_init(&_semaphore, 0, uCount);
            if (re != 0) {
                LOGE("create CSemaphore failed!");
            }
        }

        virtual ~CSemaphore() {
            sem_destroy(&_semaphore);
        }

        inline virtual void wait() {
            sem_wait(&_semaphore);
        }

        inline virtual void post() {
            sem_post(&_semaphore);
        }

    private:

        CSemaphore(const CSemaphore &) {
        }

        sem_t _semaphore;
    };

    class ILock {
    public:
        ILock() {}

        virtual ~ILock() {}

        virtual void lock() = 0;

        virtual void unlock() = 0;

        virtual void wait() = 0;

        virtual void wait(s64 microsecond) = 0;

        virtual void acttive() = 0;

        virtual void acttiveall() = 0;
    };

    class CLock : public ILock {
    public:
        CLock() {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);

            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

            pthread_mutex_init(&_lock, &attr);
            pthread_mutexattr_destroy(&attr);

            pthread_cond_init(&_cond, NULL);
        }

        virtual ~CLock() {
            pthread_mutex_destroy(&_lock);
            pthread_cond_destroy(&_cond);
        }

        inline virtual void lock() {
            if (0 != pthread_mutex_lock(&_lock)) {
                //  if( errno == EDEADLK )
                //  {
                //      assert( false );
                //  }
            }
        }

        inline virtual void unlock() {
            pthread_mutex_unlock(&_lock);
        }

        inline virtual void wait() {
            pthread_cond_wait(&_cond, &_lock);
        }

        inline virtual void wait(s64 microsecond) {
            struct timespec time;
            struct timeval now;
            gettimeofday(&now, NULL);
            s64 t = microsecond + now.tv_usec;
            time.tv_sec = (time_t) (now.tv_sec + t / 1000000);
            time.tv_nsec = (long) ((t % 1000000) * 1000);
            pthread_cond_timedwait(&_cond, &_lock, &time);
        }

        inline virtual void acttive() {
            pthread_cond_signal(&_cond);
        }

        inline virtual void acttiveall() {
            pthread_cond_broadcast(&_cond);
        }

    private:
        CLock(const CLock &) {
        }

        pthread_mutex_t _lock;
        pthread_cond_t _cond;
    };

    class CAutoLockTrace {
    public:
        CAutoLockTrace(ILock *pLock, const char *szFile, const char *szFunction, s32 nLine)
                : m_szFile(szFile),
                  m_szFunction(szFunction) {
            if (pLock != NULL) {
                m_pLock = pLock;
                m_pLock->lock();
                if (errno == EDEADLK) {
                    ALOG(LOG_VERBOSE, szFile, "[%s@%d] lock acquisition failed", szFunction, nLine);
                } else {
                    ALOG(LOG_VERBOSE, szFile, "[%s@%d] lock acquired", szFunction, nLine);
                }
            }

        }

        ~CAutoLockTrace() {
            if (m_pLock != NULL)
                m_pLock->unlock();
            ALOG(LOG_VERBOSE, m_szFile, "[%s] lock released", m_szFunction);
        }

    private:
        ILock *m_pLock;
        const char *m_szFile;
        const char *m_szFunction;
    };

    class CAutoLock {
    public:
        CAutoLock(ILock *pLock) {
            if (pLock != NULL) {
                m_pLock = pLock;
                m_pLock->lock();
            }
        }

        ~CAutoLock() {
            if (m_pLock != NULL)
                m_pLock->unlock();
        }

    private:
        ILock *m_pLock;
    };

} // namespace paomiantv

#endif // _PAOMIANTV_AUTOLOCK_H_