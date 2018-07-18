/*******************************************************************************
 *        Module: common
 *          File:
 * Functionality: safe queue defination
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 PAOMIANTV, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-17  v1.0        huangxuefeng  created
 ******************************************************************************/

#ifndef _PAOMIANTV_SAFEQUQUE_H_
#define _PAOMIANTV_SAFEQUQUE_H_

#include <queue>
#include "autolock.h"
#include "constant.h"

namespace paomiantv {
    template<class T>
    class CSafeQueue {
    public:
        CSafeQueue(const u16 max_size = BLOCK_QUEUE_SIZE, const s8 *name = NULL) :
                m_wCapacity(max_size),
                m_pchName(name),
                m_wSize(0),
                m_bDestroy(FALSE) {
            m_pLock = new CLock;
        }

        ~CSafeQueue() {
            clear();
            delete m_pLock;
            m_pLock = NULL;
        }

        bool push(T *node) {
            bool re = false;
            if (m_bDestroy) {
                return re;
            }
            m_pLock->lock();
            while (m_wSize == m_wCapacity && !m_bDestroy) {
                if (m_pchName != NULL) {
                    LOGI("the queue (%s) is full, wait for a black!", m_pchName);
                }
                m_pLock->wait(3000000);
            }
            if (m_wSize < m_wCapacity && !m_bDestroy) {
                m_vBuffers.push(node);
                m_wSize++;
                re = true;
                m_pLock->acttive();
            }
            m_pLock->unlock();
            return re;
        }

//        bool try_push(T *node) {
//            bool re = false;
//            if (m_bDestroy) {
//                return re;
//            }
//            m_pLock->lock();
//            if (m_wSize < m_wCapacity && !m_bDestroy) {
//                m_vBuffers.push(node);
//                m_wSize++;
//                re = true;
//                m_pLock->acttive();
//            }
//            m_pLock->unlock();
//            return re;
//        }

        bool pop(T *&node) {
            bool re = false;
            if (m_bDestroy) {
                return re;
            }
            m_pLock->lock();
            while (m_wSize == 0 && !m_bDestroy) {
                if (m_pchName != NULL) {
                    LOGI("the queue (%s) is empty, wait for a data!", m_pchName);
                }
                m_pLock->wait(3000000);
            }
            if (m_wSize > 0 && !m_vBuffers.empty() && !m_bDestroy) {
                node = m_vBuffers.front();
                m_vBuffers.pop();
                m_wSize--;
                re = true;
                m_pLock->acttive();
            }
            m_pLock->unlock();
            return re;
        }

//        bool try_pop(T *&node) {
//            bool re = false;
//            if (m_bDestroy) {
//                return re;
//            }
//            m_pLock->lock();
//            if (m_wSize > 0 && !m_vBuffers.empty() && !m_bDestroy) {
//                node = m_vBuffers.front();
//                m_vBuffers.pop();
//                m_wSize--;
//                re = true;
//                m_pLock->acttive();
//            }
//            m_pLock->unlock();
//            return re;
//        }

        bool empty() {
            if (m_bDestroy) {
                return TRUE;
            }
            CAutoLock cAutoLock(m_pLock);
            return m_wSize == 0;
        }

        u16 size() {
            if (m_bDestroy) {
                return 0;
            }
            CAutoLock cAutoLock(m_pLock);
            return m_wSize;
        }

        void disable() {
            CAutoLock cAutoLock(m_pLock);
            m_bDestroy = TRUE;
            m_pLock->acttiveall();
        }

        void enable() {
            CAutoLock cAutoLock(m_pLock);
            m_bDestroy = FALSE;
            m_pLock->acttiveall();
        }

        void clear() {
            T *tmp = NULL;
            m_bDestroy = TRUE;
            m_pLock->acttiveall();
            m_pLock->lock();
            while (!m_vBuffers.empty()) {
                tmp = m_vBuffers.front();
                m_vBuffers.pop();
                if (tmp != NULL) {
                    m_pLock->unlock();
                    T::release(tmp);
                    m_pLock->lock();
                }
            }
            m_wSize = 0;
            m_pLock->unlock();
            m_pLock->acttiveall();
            return;
        }


    private:
        const u16 m_wCapacity;
        const s8 *m_pchName;

        ILock *m_pLock;
        std::queue<T *> m_vBuffers;
        u16 m_wSize;
        BOOL32 m_bDestroy;

    };
}

#endif //_PAOMIANTV_SAFEQUQUE_H_
