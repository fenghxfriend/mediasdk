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

#include <frame.h>
#include "exporter.h"

namespace paomiantv {

    CExporter::CExporter()
            : m_ppStoryboard(),
              m_cbOnWrite(NULL),
              m_cbDelegate(NULL),
              m_cbOnMessage(NULL) {
        USE_LOG;
        m_pRender = new CRenderer;
        m_pVController = new CVController;
        m_pAudioTrack = new CAudioTrack;
        m_pAController = new CAController;
    }

    CExporter::~CExporter() {
        USE_LOG;
        stop();
        if (m_pAController != NULL) {
            delete m_pAController;
            m_pAController = NULL;
        }
        if (m_pAudioTrack != NULL) {
            delete m_pAudioTrack;
            m_pAudioTrack = NULL;
        }
        if (m_pVController != NULL) {
            delete m_pVController;
            m_pVController = NULL;
        }
        if (m_pRender != NULL) {
            delete m_pRender;
            m_pRender = NULL;
        }
        m_ppStoryboard = NULL;
        m_bIsPlaying = FALSE;
        CFrameManager::clearFrame();
        CSoundManager::clearSound();
        CImageManager::clearImage();
    }

    void CExporter::start() {

        if (m_pAController != NULL) {
            m_pAController->start();
        }
        if (m_pVController != NULL) {
            m_pVController->start();
        }
    }

    void CExporter::stop() {
        if (m_pAController != NULL) {
            m_pAController->stop();
        }
        if (m_pVController != NULL) {
            m_pVController->stop();
        }
    }

    void CExporter::setDataSource(CStoryboard **ppStoryboard) {
        if (ppStoryboard == NULL || *ppStoryboard == NULL) {
            LOGE("the storyboard is invalid!");
            return;
        }
        m_ppStoryboard = ppStoryboard;
        m_pVController->init(ppStoryboard, &m_pRender);
        m_pAController->init(ppStoryboard, &m_pAudioTrack);
        m_pAController->prepare(FALSE, TRUE);
        m_pVController->prepare(FALSE, TRUE);
    }

    void CExporter::bindSurface(EGLNativeWindowType pWindow, u32 uWidth, u32 uHeight) {
        m_pRender->bindEGLNativeWindow(pWindow,uWidth,uHeight);
    }

    void CExporter::setOnCB(void *cbDelegate, OnMessageCB cbOnMessage, OnWritePCMCB cbOnWrite) {
        m_cbOnMessage = cbOnMessage;
        m_cbOnWrite = cbOnWrite;
        m_cbDelegate = cbDelegate;
        m_pVController->setOnMessageCB(cbOnMessage, cbDelegate);
        m_pAController->setOnMessageCB(cbOnMessage, cbDelegate);
        m_pRender->setOnMessageCB(cbOnMessage, cbDelegate);
        m_pAudioTrack->setOnMessageCB(cbOnMessage, cbOnWrite, cbDelegate);
    }

    BOOL32
    CExporter::setVideoEncoderParams(s8 *string, jint i, jint i1, jint i2, jint i3, jint i4,
                                     jint i5, jint i6) {
        return 0;
    }

    BOOL32 CExporter::setAudioEncoderParams(s8 *string, jint i, jint i1, jint i2, jint i3) {
        return 0;
    }
}