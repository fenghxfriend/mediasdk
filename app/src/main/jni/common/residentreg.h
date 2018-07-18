#ifndef _PAOMIANTV_RESIDENTREG_H_
#define _PAOMIANTV_RESIDENTREG_H_

#include <set>
#include <pthread.h>


namespace paomiantv {


// resident registry
    class CResidentReg {
    private:
        std::set<void *> m_registry;
        pthread_mutex_t m_mutex;
    public:
        CResidentReg() {
            pthread_mutex_init(&m_mutex, 0);
        }

        ~CResidentReg() {
            pthread_mutex_destroy(&m_mutex);
        }

        void Add(void *p) {
            pthread_mutex_lock(&m_mutex);
            m_registry.insert(p);
            pthread_mutex_unlock(&m_mutex);
        }

        void Remove(void *p) {
            pthread_mutex_lock(&m_mutex);
            std::set<void *>::iterator it = m_registry.find(p);
            if (it != m_registry.end()) {
                m_registry.erase(it);
            }
            pthread_mutex_unlock(&m_mutex);
        }

        bool Contains(void *p) {
            bool found = false;
            pthread_mutex_lock(&m_mutex);
            found = m_registry.find(p) != m_registry.end();
            pthread_mutex_unlock(&m_mutex);
            return found;
        }

        void *next() {
            void *p = NULL;
            pthread_mutex_lock(&m_mutex);
            std::set<void *>::iterator it = m_registry.begin();
            while (it != m_registry.end()) {
                if (*it != NULL) {
                    p = *it;
                    m_registry.erase(it);
                    break;
                }
            }
            pthread_mutex_unlock(&m_mutex);
            return p;
        }

    };

} // namespace paomiantv


#endif // _PAOMIANTV_RESIDENTREG_H_