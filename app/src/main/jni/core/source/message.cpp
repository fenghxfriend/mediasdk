//
// Created by ASUS on 2018/6/12.
//

#include <core.h>
#include <eventmanager.h>
#include "message.h"

namespace paomiantv {

    std::vector<CMessage *> CMessage::m_svPool;
    CLock CMessage::m_sLock;

    CMessage *CMessage::create() {
        CMessage *re = NULL;
        m_sLock.lock();
        if (!m_svPool.empty()) {
            re = m_svPool.back();
            m_svPool.pop_back();
        } else {
            re = new CMessage();
        }
        m_sLock.unlock();
        return re;
    }

    void CMessage::release(CMessage *message) {
        m_sLock.lock();
        if (message != NULL) {
            message->reset();
            std::vector<CMessage *>::iterator it;
            it = std::find(m_svPool.begin(), m_svPool.end(), message);
            if (it == m_svPool.end()) {
                //vec中不存在value值
                m_svPool.push_back(message);
            }
        }
        m_sLock.unlock();
    }

    void CMessage::clear() {
        m_sLock.lock();
        while (!m_svPool.empty()) {
            CMessage *message = m_svPool.back();
            m_svPool.pop_back();
            if (message != NULL) {
                delete message;
            }
        }
        m_svPool.clear();
        m_sLock.unlock();
    }


    CMessage::CMessage() : m_uSrcId(0),
                           m_uTarId(0),
                           m_nId(EM_EVENT_START),
                           m_uSize(0),
                           m_pData(NULL) {

    }

    CMessage::~CMessage() {
        if (m_pData != NULL) {
            free(m_pData);
            m_pData = NULL;
        }
    }

    void CMessage::reset() {
        m_uSrcId = 0;
        m_uTarId = 0;
        m_nId = EM_EVENT_START;
        m_uSize = 0;
        if (m_pData != NULL) {
            free(m_pData);
            m_pData = NULL;
        }
    }

    bool CMessage::send(u32 uSrcId, u32 uTarId, s32 nId, u32 uSize, void *pData) {
        if (validate(uSrcId, uTarId, nId, uSize)) {
            this->m_uSrcId = uSrcId;
            this->m_uTarId = uTarId;
            this->m_nId = nId;
            this->m_uSize = uSize;
            this->m_pData = pData;
            return CCore::getInstance()->sendMessage(this);
        }
        return false;
    }

    bool CMessage::validate(u32 srcId, u32 tarId, s32 id, u32 size) {
        return (CCore::getInstance()->checkModule(srcId)
                && CCore::getInstance()->checkModule(tarId)
                && EM_EVENT_START < id && id < EM_EVENT_END
                && size <= MAX_MESSAGE_CONTENT
                && CEventManager::getInstance()->validateContent((EM_EVENT) id, size));
    }


}

