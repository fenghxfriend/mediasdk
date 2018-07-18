//
// Created by ASUS on 2018/6/14.
//

#ifndef MEDIAENGINE_EVENTMANAGER_H
#define MEDIAENGINE_EVENTMANAGER_H

#include <typedef.h>
#include "event.h"

namespace paomiantv {

    typedef struct tagEvent {
        EM_EVENT event;
        u32 size;
        const s8 *description;
    } TEvent;

#define REGISTER_EVENT(event, contentSize, eventDescription)  \
    { \
        m_tEvent[event].size = contentSize; \
        m_tEvent[event].description = eventDescription; \
    }


    class CEventManager {
    private:
        CEventManager();

        virtual ~CEventManager();


        static CEventManager *m_pInstance;

        class Garbo {
        public:
            ~Garbo() {
                if (CEventManager::m_pInstance) {
                    delete CEventManager::m_pInstance;
                }
            }
        };

        static Garbo garbo;
        TEvent m_tEvent[EM_EVENT_END];

        void registerEvent();

    public:
        static CEventManager *getInstance();

        u32 getSize(EM_EVENT event);

        const s8 *getDescription(EM_EVENT event);

        bool validateContent(EM_EVENT event, u32 size);
    };
}

#endif //MEDIAENGINE_EVENTMANAGER_H
