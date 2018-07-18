//
// Created by ASUS on 2018/6/14.
//

#include "eventmanager.h"
#include <typedef.h>

namespace paomiantv {
    CEventManager::Garbo CEventManager::garbo; // 一定要初始化，不然程序结束时不会析构garbo

    CEventManager *CEventManager::m_pInstance = NULL;

    CEventManager *CEventManager::getInstance() {
        if (m_pInstance == NULL)
            m_pInstance = new CEventManager();
        return m_pInstance;
    }

    CEventManager::CEventManager() {
        registerEvent();
    }

    CEventManager::~CEventManager() {

    }

    void CEventManager::registerEvent() {
        REGISTER_EVENT(EM_EVENT_TEST, 0, "this event is just for test");
    }

    u32 CEventManager::getSize(EM_EVENT event) {
        return m_tEvent[event].size;
    }

    const s8 *CEventManager::getDescription(EM_EVENT event) {
        return m_tEvent[event].description;
    }

    bool CEventManager::validateContent(EM_EVENT event, u32 size) {
        return m_tEvent[event].size == size;
    }
}