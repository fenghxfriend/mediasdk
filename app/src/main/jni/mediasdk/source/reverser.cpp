/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: exporter
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-03  v1.0        huangxuefeng  created
 ******************************************************************************/

#include "reverser.h"

namespace paomiantv {

    CReverser::CReverser()
            : m_cbOnWritePCM(NULL),
              m_cbDelegate(NULL),
              m_cbOnMessage(NULL),
              m_cbOnStartVEncode(NULL),
              m_cbOnStartAEncode(NULL) {
        USE_LOG;
        m_pDemuxer = new CDemuxer;
        m_pRender = new CRenderer;
        m_pAudioTrack = new CAudioTrack;
        m_pRevController = new CRevController;
    }

    CReverser::~CReverser() {
        USE_LOG;
        stop();
        if (m_pDemuxer != NULL) {
            delete m_pDemuxer;
            m_pDemuxer = NULL;
        }
        if (m_pRender != NULL) {
            delete m_pRender;
            m_pRender = NULL;
        }
        if (m_pAudioTrack != NULL) {
            delete m_pAudioTrack;
            m_pAudioTrack = NULL;
        }
        if (m_pRevController != NULL) {
            delete m_pRevController;
            m_pRevController = NULL;
        }
        CFrameManager::clearFrame();
        CSoundManager::clearSound();
        CImageManager::clearImage();
    }

    void CReverser::start() {
        if (m_pRevController != NULL) {
            m_pRevController->start();
        }
    }

    void CReverser::stop() {
        if (m_pRevController != NULL) {
            m_pRevController->stop();
        }
    }

    void CReverser::reverse(CMuxer *muxer,s8 *tempPath, s8 *path, BOOL32 isVideoReverse, BOOL32 isAudioReverse) {
        if (path == NULL || tempPath == NULL) {
            LOGE("the path is invalid!");
            return;
        }
        if (m_pDemuxer->init(path)) {
            m_pRevController->init(&m_pDemuxer,&m_pAudioTrack, &m_pRender);
            m_pRevController->prepare(muxer,tempPath,isVideoReverse, isAudioReverse);
            m_pRevController->start();
        } else {
            LOGE("init demuxer failed!");
        }
    }

    void CReverser::bindSurface(EGLNativeWindowType pWindow, u32 uWidth, u32 uHeight) {
        m_pRender->bindEGLNativeWindow(pWindow, uWidth, uHeight);
    }

    void CReverser::setOnCB(void *cbDelegate, OnMessageCB cbOnMessage, OnWritePCMCB cbOnWrite,
                            OnStartVEncodeCB cbStartVEncodeCB,
                            OnStartAEncodeCB cbStartAEncodeCB) {
        m_cbOnMessage = cbOnMessage;
        m_cbOnWritePCM = cbOnWrite;
        m_cbDelegate = cbDelegate;
        m_cbOnStartVEncode = cbStartVEncodeCB;
        m_cbOnStartAEncode = cbStartAEncodeCB;
        m_pRevController->setOnMessageCB(cbOnMessage,cbDelegate);
        m_pRender->setOnMessageCB(cbOnMessage, cbDelegate);
        m_pAudioTrack->setOnMessageCB(cbOnMessage, cbOnWrite, cbDelegate);
        m_pRevController->setOnEncodeThread(cbStartVEncodeCB,cbStartAEncodeCB,cbDelegate);
    }
}