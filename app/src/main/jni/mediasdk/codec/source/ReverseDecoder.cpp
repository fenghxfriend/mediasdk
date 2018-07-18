//
// Created by ASUS on 2018/6/1.
//

#include "../include/ReverseDecoder.h"

namespace paomiantv {


    ReverseDecoder::ReverseDecoder(CDemuxer *demuxer) :
            m_pDemuxer(demuxer),
            m_bIsStopped(FALSE),
            m_bIsStarted(FALSE),
            m_bIsFinished(FALSE) {
        USE_LOG;
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
    }

    ReverseDecoder::~ReverseDecoder() {
        USE_LOG;
        stop();
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

    void *ReverseDecoder::ThreadWrapper(void *pData) {
        ReverseDecoder *p = (ReverseDecoder *) pData;
        return (void *) p->run();
    }


    void ReverseDecoder::start() {
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

    void ReverseDecoder::stop() {
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
}
