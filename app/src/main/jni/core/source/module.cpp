//
// Created by ASUS on 2018/6/12.
//

#include <constant.h>
#include <autolog.h>
#include <cstring>
#include <core.h>
#include "module.h"

namespace paomiantv {
    u32 IModule::m_sCount = 1;

    IModule::IModule(const s8 *name) :
            m_uId(m_sCount),
            m_cbHandler(NULL) {
        if (m_sCount == UNSIGNED_INTEGER32_MAX_VALUE) {
            LOGE("the id is overflowed! please restart app!");
        } else {
            m_sCount++;
        }
        m_pLockHandler = new CLock;
        init(name);
    }

    IModule::~IModule() {
        uninit();
        if (m_pLockHandler != NULL) {
            delete m_pLockHandler;
            m_pLockHandler = NULL;
        }
    }

    void IModule::init(const s8 *name) {
        setName(name);
        CCore::getInstance()->registerModule(this);
    }

    void IModule::uninit() {

        CCore::getInstance()->unregisterModule(this);
    }

    void IModule::setName(const s8 *name) {
        if (name != NULL) {
            strncpy(m_achName, name, MAX_MODULE_NAME_LEN - 1);
            m_achName[MAX_MODULE_NAME_LEN - 1] = '\0';
        } else {
            memset(m_achName, 0, MAX_MODULE_NAME_LEN);
        }
    }

    void IModule::setMessageCallback(OnMessageCallback callback) {
        CAutoLock autoLock(m_pLockHandler);
        this->m_cbHandler = callback;
    }

    bool IModule::handleMessage(CMessage *message) {
        CAutoLock autoLock(m_pLockHandler);
        if (this->m_cbHandler != NULL) {
            return this->m_cbHandler(message);
        }
        return true;
    }
}

