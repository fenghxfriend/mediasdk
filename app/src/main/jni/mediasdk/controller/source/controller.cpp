/*******************************************************************************
 *        Module: mediasdk
 *          File: acontroller.cpp
 * Functionality: handle audio data.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-01  v1.0        huangxuefeng  created
 ******************************************************************************/

#include <constant.h>
#include "controller.h"

static const size_t s_defaultCacheCount = 5;

namespace paomiantv {

    CController::CController() :
            m_cbOnMessage(NULL),
            m_cbDelegate(NULL),
            m_bIsStarted(FALSE),
            m_bIsStopped(FALSE),
            m_bIsPaused(FALSE),
            m_sllStartTS(0),
            m_sllCurTS(0),
            m_pTrackList(NULL),
            m_cacheReadyCallback(NULL),
            m_sllEndTS(INTEGER64_MAX_VALUE) {
        USE_LOG;
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
        m_cacheCount = s_defaultCacheCount;
        m_cacheNumber = 0;
    }

    CController::~CController() {
        USE_LOG;
        if (m_pThread != NULL) {
            m_pThread->join();
            delete m_pThread;
            m_pThread = NULL;
        }
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
    }

    void *CController::ThreadWrapper(void *pData) {
        CController *p = (CController *) pData;
        return (void *) p->run();
    }

    void CController::setOnMessageCB(OnMessageCB cbOnMessage, void *cbDelegate) {
        CAutoLock autoLock(m_pLock);
        m_cbOnMessage = cbOnMessage;
        m_cbDelegate = cbDelegate;
    }

    void CController::sendErrorMessage(s32 id) {
        CAutoLock autoLock(m_pLock);
        if (m_cbDelegate != NULL && m_cbOnMessage != NULL) {
            m_cbOnMessage(m_cbDelegate, id, NULL);
        }
    }

    void CController::start() {
        LOGI("controller::startThread");
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted) {
            return;
        }
        if (!m_pThread->start()) {
            LOGE("start controller thread failed!");
            return;
        }
    }

    void CController::stop() {
        char name[64];
        memset(name, 0, sizeof(name));
        m_pThread->getName(name);
        LOGI("%s::stopThread", name);
        m_pLock->lock();
        if (m_bIsStarted && !m_bIsStopped) {
            m_bIsStopped = TRUE;
            m_pLock->acttive();
        }
        m_pLock->unlock();
        m_pThread->join();
    }

    void CController::resume() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && m_bIsPaused) {
            m_bIsPaused = FALSE;
            m_pLock->acttive();
        }
    }

    void CController::pause() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && !m_bIsPaused) {
            m_bIsPaused = TRUE;
        }
    }

    void CController::_increaseCacheNumber() {
        if (m_cacheNumber < m_cacheCount) {
            ++m_cacheNumber;
            if (m_cacheNumber >= m_cacheCount) {
                if (m_cacheReadyCallback) {
                    m_cacheReadyCallback();
                }
            }
        }
    }

    void CController::_resetCacheNumber() {
        m_cacheNumber = 0;
    }
}