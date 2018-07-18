//
// Created by ASUS on 2018/6/12.
//
#include <typedef.h>
#include <unistd.h>
#include "core.h"

namespace paomiantv {
    CCore::Garbo CCore::garbo; // 一定要初始化，不然程序结束时不会析构garbo

    CCore *CCore::m_pInstance = NULL;

    bool CCore::compareByIdCB(u64 id, IModule *module) {
        return (id == module->getId());
    }

    CCore *CCore::getInstance() {
        if (m_pInstance == NULL)
            m_pInstance = new CCore();
        return m_pInstance;
    }

    CCore::CCore() : m_bShutDown(false) {
        m_pLockModule = new CLock;
        m_pMsgBus = new CSafeQueue<CMessage>(MESSAGE_BUS_CAPACITY, "bus line");
        m_pModuleList = new CSafeList<IModule>;
        m_pThread = new CThread(ThreadWrapper, this);
        CMessage::clear();
    }


    CCore::~CCore() {
        delete m_pMsgBus;
        delete m_pModuleList;
        delete m_pThread;
        delete m_pLockModule;
        CMessage::clear();
    }

    void CCore::setup() {
        m_pMsgBus->enable();
        m_pThread->start();
    }

    void CCore::shutdown() {
        m_bShutDown = true;
        m_pMsgBus->disable();
        m_pThread->join();
        m_pMsgBus->clear();
    }


    bool CCore::registerModule(IModule *module) {
        CAutoLock autoLock(m_pLockModule);
        if (module == NULL || m_pModuleList->contain(module)) {
            LOGE("module is invalidate ,or already registered!");
            return false;
        }
        m_pModuleList->push(module);
        return true;
    }

    void CCore::unregisterModule(IModule *module) {
        CAutoLock autoLock(m_pLockModule);
        m_pModuleList->remove(module);
    }

    bool CCore::sendMessage(CMessage *message) {
        return m_pMsgBus->push(message);
    }

    void *CCore::ThreadWrapper(void *pData) {
        CCore *p = (CCore *) pData;
        return (void *) p->run();
    }

    long CCore::run() {
        while (!m_bShutDown) {
            CMessage *message = NULL;
            m_pMsgBus->pop(message);
            if (message != NULL) {
                handleMessage(message);
            } else {
                LOGW("message is invalid!");
            }
        }
        return 0;
    }

    bool CCore::checkModule(u32 id) {
        return m_pModuleList->contain(id, compareByIdCB);
    }

    void CCore::handleMessage(CMessage *message) {
        CAutoLock autoLock(m_pLockModule);
        IModule *dst = m_pModuleList->get(message->m_uTarId, compareByIdCB);
        if (dst != NULL) {
            dst->handleMessage(message);
        } else {
            LOGW("target is invalid, drop it!");
            CMessage::release(message);
        }
    }
}


