#ifndef _PAOMIANTV_THREAD_H_
#define _PAOMIANTV_THREAD_H_

#include <pthread.h>
#include <sys/prctl.h>
#include <typeinfo>
#include "autolog.h"

namespace paomiantv {

    typedef void *(*ThreadTask)(void *);


//    typedef struct tagThreadData {
//        //! the delegate of callback.
//        void *holder;
//        //! the task need callback(ThreadTask).
//        void *task;
//
//        //! constructure
//        tagThreadData() {
//            holder = NULL;
//            task = NULL;
//        }
//
//        tagThreadData(void *h, void *t) {
//            holder = h;
//            task = t;
//        }
//    } ThreadData;

    typedef enum {
        EM_THREAD_STATUS_UNKNOWN,
        EM_THREAD_STATUS_STARTED,
        EM_THREAD_STATUS_JOINED
    } EMThreadStatus;

    class ILock;

    class CAutoLock;

    class CThread {
    public:
        CThread(ThreadTask task, void *holder)
                : m_eStatus(EM_THREAD_STATUS_UNKNOWN),
                  m_pHolder(holder),
                  m_pTask(task),
                  m_thread(0) {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);

            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

            pthread_mutex_init(&m_lock, &attr);
            pthread_mutexattr_destroy(&attr);
        }

        virtual ~CThread() {
            pthread_mutex_destroy(&m_lock);
        }

        void setName(const char *pchName) {
#ifndef __ANDROID__
            prctl(PR_SET_NAME, pchName);
#else
            prctl(PR_SET_NAME, (unsigned long) pchName, 0, 0, 0);
#endif
        }

        void getName(const char *pchName) {
#ifndef __ANDROID__
            prctl(PR_GET_NAME, pchName);
#else
            prctl(PR_GET_NAME, (unsigned long) pchName, 0, 0, 0);
#endif
        }

        BOOL32 start() {
            BOOL32 ret = TRUE;
            pthread_mutex_lock(&m_lock);
            do {
                if (m_eStatus == EM_THREAD_STATUS_STARTED) {
                    LOGW("thread is already running!");
                    ret = FALSE;
                    break;
                }
                int nErr;
                pthread_attr_t attr;
                nErr = pthread_attr_init(&attr);
                if (nErr) {
                    LOGE("startThread failed, jni_init thread attribute failed!");
                    ret = FALSE;
                    break;
                }

                nErr = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
                if (nErr) {
                    LOGE("startThread failed, set thread detach state failed!");
                    pthread_attr_destroy(&attr);
                    ret = FALSE;
                    break;
                }
                if (m_pHolder == NULL && m_pTask == NULL) {
                    pthread_attr_destroy(&attr);
                    ret = FALSE;
                    break;
                }

                nErr = pthread_create(&m_thread, &attr, (void *(*)(void *)) m_pTask,
                                      const_cast<void *>(m_pHolder));
                pthread_attr_destroy(&attr);
                if (nErr) {
                    LOGE("startThread failed, create thread failed!");
                    ret = FALSE;
                    break;
                }
            } while (0);
            if (ret) {
                m_eStatus = EM_THREAD_STATUS_STARTED;
            }
            pthread_mutex_unlock(&m_lock);
            return ret;
        }

        void *join() {
            USE_LOG;
            void *p = NULL;
            pthread_mutex_lock(&m_lock);
            do {
                if (m_eStatus != EM_THREAD_STATUS_STARTED) {
                    LOGW("thread is not running!");
                    break;
                }

                int nErr = pthread_join(m_thread, (void **) &p);
                if (nErr) {
                    LOGE("thread is not started errNO: %d", nErr);
                }
                m_thread = 0;
                m_eStatus = EM_THREAD_STATUS_JOINED;
            } while (0);
            pthread_mutex_unlock(&m_lock);
            return p;

        }

    private:
        pthread_t m_thread;
        pthread_mutex_t m_lock;
        const void *m_pHolder;
        const ThreadTask m_pTask;
        EMThreadStatus m_eStatus;
    };

} // namespace paomiantv

#endif // _PAOMIANTV_THREAD_H_