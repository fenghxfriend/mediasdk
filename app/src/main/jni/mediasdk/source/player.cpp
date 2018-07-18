//
// Created by ASUS on 2018/2/8.
//

#include <player.h>
#include <vcontroller.h>
#include <acontroller.h>
#include <frame.h>

namespace paomiantv {
    CPlayer *CPlayer::Create() {
        CPlayer *p = new CPlayer;
        if (p->init() != 0) {
            delete p;
            return NULL;
        }
        return p;
    }

    CPlayer::CPlayer() : m_eStatus(EM_PLAYER_STATUS_IDLE),
                         m_ppStoryboard(NULL),
                         m_bIsPlaying(FALSE)
//            , m_cbDelegate(NULL), m_cbOnMessage(NULL)
    {
        USE_LOG;
        m_pRender = new CRenderer;
        m_pVController = new CVController;
        m_pAudioTrack = new CAudioTrack;
        m_pAController = new CAController;

        m_pAController->setReadyCallback(std::bind(&CPlayer::onAudioReady, this));
        m_pVController->setReadyCallback(std::bind(&CPlayer::onVideoReady, this));
        m_isAudioReady = false;
        m_isVideoReady = false;
        m_isAudioPreparing = false;
        m_isVideoPreparing = false;

        m_playerLock = new CLock();
    }

    CPlayer::~CPlayer() {
        USE_LOG;
        stop();
        if (m_pAController != NULL) {
            m_pAController->setReadyCallback(nullptr);
            delete m_pAController;
            m_pAController = NULL;
        }
        if (m_pAudioTrack != NULL) {
            delete m_pAudioTrack;
            m_pAudioTrack = NULL;
        }
        if (m_pVController != NULL) {
            m_pVController->setReadyCallback(nullptr);
            delete m_pVController;
            m_pVController = NULL;
        }
        if (m_pRender != NULL) {
            delete m_pRender;
            m_pRender = NULL;
        }
        m_ppStoryboard = NULL;
        m_bIsPlaying = FALSE;

        m_eStatus = EM_PLAYER_STATUS_IDLE;

        if (m_playerLock) {
            delete m_playerLock;
            m_playerLock = nullptr;
        }
        CFrameManager::clearFrame();
        CSoundManager::clearSound();
        CImageManager::clearImage();
    }

    int CPlayer::init() {
        return 0;
    }

    void CPlayer::setDataSource(CStoryboard **ppStoryboard) {
        CAutoLock autoLock(m_playerLock);
        switch (m_eStatus) {
            case EM_PLAYER_STATUS_IDLE:
                break;
            default:
                LOGW("player.prepare:Invalid player status = %d", m_eStatus);
                return;
        }
        if (ppStoryboard == NULL || *ppStoryboard == NULL) {
            LOGE("the storyboard is invalid!");
            return;
        }
        m_ppStoryboard = ppStoryboard;
        m_pVController->init(ppStoryboard, &m_pRender);
        m_pAController->init(ppStoryboard, &m_pAudioTrack);
    };

    void CPlayer::prepare() {
        CAutoLock autoLock(m_playerLock);
        switch (m_eStatus) {
            case EM_PLAYER_STATUS_IDLE:
                break;
            default:
                LOGW("player.prepare:Invalid player status = %d", m_eStatus);
                return;
        }
        if (!m_ppStoryboard || !*m_ppStoryboard) {
            LOGE("player.prepare:Null storyboard!");
            return;
        }
        m_eStatus = EM_PLAYER_STATUS_PREPARED;
        m_isVideoReady = false;
        m_isAudioReady = false;
        m_pAController->prepare();
        m_pVController->prepare();
    }

    void CPlayer::start() {
        CAutoLock autoLock(m_playerLock);
        switch (m_eStatus) {
            case EM_PLAYER_STATUS_PREPARED:
                break;
            default:
                LOGW("player.start:Invalid player status = %d", m_eStatus);
                return;
        }
        m_eStatus = EM_PLAYER_STATUS_STARTED;
        m_pAController->start();
        m_pVController->start();
    }

    void CPlayer::pause() {
        CAutoLock autoLock(m_playerLock);
        switch (m_eStatus) {
            case EM_PLAYER_STATUS_STARTED:
                break;
            default:
                LOGW("player.pause:Invalid player status = %d", m_eStatus);
                return;
        }
        m_eStatus = EM_PLAYER_STATUS_PAUSED;
        m_pAController->pause();
        m_pVController->pause();
    }

    void CPlayer::resume() {
        CAutoLock autoLock(m_playerLock);
        switch (m_eStatus) {
            case EM_PLAYER_STATUS_PAUSED:
                break;
            default:
                LOGW("player.resume:Invalid player status = %d", m_eStatus);
                return;
        }
        m_eStatus = EM_PLAYER_STATUS_STARTED;
        m_pAController->resume();
        m_pVController->resume();
    }

    void CPlayer::stop() {
        CAutoLock autoLock(m_playerLock);
        switch (m_eStatus) {
            case EM_PLAYER_STATUS_PAUSED:
            case EM_PLAYER_STATUS_STARTED:
                break;
            default:
                LOGW("player.stop:Invalid player status = %d", m_eStatus);
                return;
        }
        m_eStatus = EM_PLAYER_STATUS_PREPARED;
        m_pAController->stop();
        m_pVController->stop();
    }

    void CPlayer::setOnMessageCB(OnMessageCB cbOnMessage, void *cbDelegate) {
//        m_cbOnMessage = cbOnMessage;
//        m_cbDelegate = cbDelegate;
        CAutoLock autoLock(m_playerLock);
        m_pVController->setOnMessageCB(cbOnMessage, cbDelegate);
        m_pAController->setOnMessageCB(cbOnMessage, cbDelegate);
        m_pRender->setOnMessageCB(cbOnMessage, cbDelegate);
        m_pAudioTrack->setOnMessageCB(cbOnMessage, NULL, cbDelegate);
    }

    void CPlayer::bindSurface(EGLNativeWindowType pWindow, u32 uWidth, u32 uHeight) {
        m_pRender->bindEGLNativeWindow(pWindow, uWidth, uHeight);
    }

    void CPlayer::surfaceSizeChanged(u32 uGLWidth, u32 uGLHeight) {
        if (m_pRender == NULL) {
            LOGE("renderer is null or released!");
            return;
        }
        m_pRender->updateGLRect(uGLWidth, uGLHeight);
    }

    void CPlayer::unbindSurface(void (*releaseWindow)(EGLNativeWindowType)) {
        if (m_pRender != NULL) {
            EGLNativeWindowType m_eglWin = m_pRender->getEGLWin();
            m_pRender->bindEGLNativeWindow(NULL, 0, 0);
            if (releaseWindow != NULL) {
                releaseWindow(m_eglWin);
            }
        }
    }

    void CPlayer::seekTo(s64 sllMicrosecond) {
        CAutoLock autoLock(m_playerLock);
        switch (m_eStatus) {
            case EM_PLAYER_STATUS_STARTED:
            case EM_PLAYER_STATUS_PAUSED:
                break;
            default:
                LOGW("player.seekTo:Invalid player status = %d", m_eStatus);
                return;
        }
        if (m_isAudioPreparing || m_isVideoPreparing) {
            return;
        }

        m_isAudioPreparing = true;
        m_isVideoPreparing = true;
        m_isAudioReady = false;
        m_isVideoReady = false;

        if (!m_pAController->seekTo(sllMicrosecond)) {
            m_isAudioReady = true;
            m_isAudioPreparing = false;
        }
        if (!m_pVController->seekTo(sllMicrosecond)) {
            m_isVideoReady = true;
            m_isVideoPreparing = false;
        }
        m_eStatus = EM_PLAYER_STATUS_STARTED;
    }

    void CPlayer::locPreview(s64 sllMicrosecond) {
        CAutoLock autoLock(m_playerLock);
        switch (m_eStatus) {
            case EM_PLAYER_STATUS_STARTED:
            case EM_PLAYER_STATUS_PAUSED:
                break;
            default:
                LOGW("player.locPreview:Invalid player status = %d", m_eStatus);
                return;
        }
        if (m_isAudioPreparing || m_isVideoPreparing) {
            return;
        }

        m_isAudioPreparing = true;
        m_isVideoPreparing = true;
        m_isAudioReady = false;
        m_isVideoReady = false;

        if (!m_pAController->locPreview(sllMicrosecond)) {
            m_isAudioReady = true;
            m_isAudioPreparing = false;
        }
        if (!m_pVController->locPreview(sllMicrosecond)) {
            m_isVideoReady = true;
            m_isVideoPreparing = false;
        }
        m_eStatus = EM_PLAYER_STATUS_PAUSED;
    }

    void CPlayer::onAudioReady() {
        m_isAudioReady = true;
        if (m_isVideoReady) {
            if (m_pRender) {
                m_pRender->setCaching(false);
                m_pRender->resume();
            }
            if (m_pAudioTrack) {
                m_pAudioTrack->setCaching(false);
                m_pAudioTrack->resume();
            }
            m_isVideoPreparing = false;
            m_isAudioPreparing = false;
            m_isAudioReady = false;
            m_isVideoReady = false;
        }
    }

    void CPlayer::onVideoReady() {
        m_isVideoReady = true;
        if (m_isAudioReady) {
            if (m_pRender) {
                m_pRender->setCaching(false);
                m_pRender->resume();
            }
            if (m_pAudioTrack) {
                m_pAudioTrack->setCaching(false);
                m_pAudioTrack->resume();
            }
            m_isVideoPreparing = false;
            m_isAudioPreparing = false;
            m_isAudioReady = false;
            m_isVideoReady = false;
        }
    }
}


