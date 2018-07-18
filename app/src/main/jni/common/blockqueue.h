/*******************************************************************************
 *        Module: common
 *          File: 
 * Functionality: queue with thread safety
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
#ifndef _PAOMIAN_BLOCK_QUEUE_H
#define _PAOMIAN_BLOCK_QUEUE_H

#include <iostream>
#include <pthread.h>
#include <sys/time.h>

namespace paomiantv {
    template<typename T>
    class CBlockQueue {
    public:
        CBlockQueue(int capacity) : m_front(-1), m_back(-1), m_size(0), m_max_size(capacity) {
            pthread_mutex_init(&m_mutex, NULL);
            pthread_cond_init(&m_cond, NULL);
            m_array = new T[capacity];
        }

        ~CBlockQueue() {

            clear();

            pthread_mutex_destroy(&m_mutex);
            pthread_cond_destroy(&m_cond);
        }

        bool full() const {
            pthread_mutex_lock(&m_mutex);
            if (m_size >= m_max_size) {
                pthread_mutex_unlock(&m_mutex);
                return true;
            }
            pthread_mutex_unlock(&m_mutex);
            return false;
        }

        bool empty() const {
            pthread_mutex_lock(&m_mutex);
            if (0 == m_size) {
                pthread_mutex_unlock(&m_mutex);
                return true;
            }
            pthread_mutex_unlock(&m_mutex);
            return false;
        }

        bool front(T &value) const {
            pthread_mutex_lock(&m_mutex);
            if (0 == m_size) {
                pthread_mutex_unlock(&m_mutex);
                return false;
            }
            value = m_array[m_front];
            pthread_mutex_unlock(&m_mutex);
            return true;
        }

        bool back(T &value) const {
            pthread_mutex_lock(&m_mutex);
            if (0 == m_size) {
                pthread_mutex_unlock(&m_mutex);
                return false;
            }
            value = m_array[m_back];
            pthread_mutex_unlock(&m_mutex);
            return true;
        }

        int size() const {
            int tmp = 0;
            pthread_mutex_lock(&m_mutex);
            tmp = m_size;
            pthread_mutex_unlock(&m_mutex);
            return tmp;
        }

        int max_size() const {
            int tmp = 0;
            pthread_mutex_lock(&m_mutex);
            tmp = m_max_size;
            pthread_mutex_unlock(&m_mutex);
            return tmp;
        }

        bool push(const T &item) {
            pthread_mutex_lock(&m_mutex);
            if (m_size >= m_max_size) {
                pthread_cond_broadcast(&m_cond);
                pthread_mutex_unlock(&m_mutex);
                return false;
            }

            m_back = (m_back + 1) % m_max_size;
            m_array[m_back] = item;

            m_size++;
            pthread_cond_broadcast(&m_cond);
            pthread_mutex_unlock(&m_mutex);

            return true;
        }

        bool pop(T &item) {
            pthread_mutex_lock(&m_mutex);
            while (m_size <= 0) {
                if (0 != pthread_cond_wait(&m_cond, &m_mutex)) {
                    pthread_mutex_unlock(&m_mutex);
                    return false;
                }
            }

            m_front = (m_front + 1) % m_max_size;
            item = m_array[m_front];
            m_size--;
            pthread_mutex_unlock(&m_mutex);
            return true;
        }

        bool pop(T &item, int ms_timeout) {
            struct timespec t = {0, 0};
            struct timeval now = {0, 0};
            gettimeofday(&now, NULL);
            pthread_mutex_lock(&m_mutex);
            if (m_size <= 0) {
                t.tv_sec = now.tv_sec + ms_timeout / 1000;
                t.tv_nsec = (ms_timeout % 1000) * 1000;
                if (0 != pthread_cond_timedwait(&m_cond, &m_mutex, &t)) {
                    pthread_mutex_unlock(&m_mutex);
                    return false;
                }
            }

            m_front = (m_front + 1) % m_max_size;
            item = m_array[m_front];
            m_size--;
            pthread_mutex_unlock(&m_mutex);
            return true;
        }

    private:
        void clear() {
            pthread_mutex_lock(&m_mutex);
            if (m_front != NULL || m_back != NULL || m_size != 0) {
            }
            pthread_mutex_unlock(&m_mutex);
        }

    private:
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;
        int m_front;
        int m_back;
        T *m_array;
        int m_size;
        const int m_max_size;

    }; //class CBlockQueue

} // namespace paomiantv
#endif