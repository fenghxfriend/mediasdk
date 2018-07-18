/*******************************************************************************
 *        Module: paomiantv
 *          File: vcontroller.cpp
 * Functionality: handle video data.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/
#include <renderer.h>
#include <vmultidec.h>
#include <multitrack.h>
#include <wpdec.h>
#include <bitmapdec.h>
#include <unistd.h>
#include "vcontroller.h"
#include <VideoDecoder.h>
#include <BitmapDecoder.h>
#include <WebpDecoder.h>
#include <VideoMultiDecoder.h>

namespace paomiantv {

    CVController::CVController() : m_ppRender(NULL) {
        USE_LOG;
        m_vDec.clear();
#if USING_VIDEO_DECODER_EX
        std::vector<IDecoder *> m_decoders;
#endif
        _seekList.clear();
    }

    CVController::~CVController() {
        USE_LOG;
        stop();
        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL) {
                delete m_vDec[i];
            }
        }
        m_vDec.clear();
#if USING_VIDEO_DECODER_EX
        std::vector<IDecoder *> deleteList;
        std::swap(m_decoders, deleteList);
        for (auto pDecoder : deleteList) {
            delete pDecoder;
        }
        deleteList.clear();
#endif
    }

    long CVController::run() {
        m_pThread->setName("CVController");
        bool isComplete = false;
        LOGI("video controller is started");
        m_bIsStarted = TRUE;

        s64 timestamp = 0;
        EMCtrlMsg msgtype = EM_CTRL_MESSAGE_START;
        while (!m_bIsStopped) {

            if (!_seekList.empty()) {
                TMessage *msg = (TMessage *) _seekList.back();
                timestamp = msg->timestamp;
                msgtype = msg->type;
                while (!_seekList.empty()) {
                    TMessage *tmp = (TMessage *) _seekList.back();
                    _seekList.pop_back();
                    delete tmp;
                }
                _seekTo(timestamp);
            }

            bool isSeeking = false;
            do {
                for (auto pDecoder : m_decoders) {
                    if (pDecoder->isSeeking()) {
                        isSeeking = true;
                        break;
                    }
                }
            } while (false);

            if (isSeeking) {
                usleep(3000);
                continue;
            }

            if (m_bIsPaused) {
                usleep(3000);
                continue;
            }

            if (m_sllCurTS < m_sllEndTS) {
                isComplete = false;

                CImage *image = CImage::create();
                image->reset();
                CVLayerParam *layer = NULL;
                for (int i = 0; i < m_vDec.size(); i++) {
                    m_vDec[i]->getLayer(layer);
                    if (layer != NULL) {
                        image->m_vVLayerParam.push_back(layer);
                    }
                }
#if USING_VIDEO_DECODER_EX
                for (size_t i = 0; i < m_decoders.size(); ++i) {
                    layer = m_decoders[i]->getImageLayer(m_sllCurTS);
                    if (layer) {
                        image->m_vVLayerParam.push_back(layer);
                    }
                }
#endif

                image->m_sllTimeStampUS = m_sllCurTS;
                m_sllCurTS += VIDEO_FRAME_MIN_DURATION;

                if (image->m_vVLayerParam.size() <= 0 ||
                    (*m_ppRender) == NULL ||
                    !(*m_ppRender)->addImage(image)) {
                    CImage::release(image);
                }

                if (msgtype == EM_CTRL_MESSAGE_LOC_PREVIEW) {
                    // seek for preview a frame should render a frame!
                    if (m_cacheReadyCallback) {
                        m_cacheReadyCallback();
                    }
                    m_bIsPaused = TRUE;
                    msgtype = EM_CTRL_MESSAGE_START;
                } else {
                    _increaseCacheNumber();
                }


            } else {
                if (!isComplete) {
                    sendImageDecodeCompleteMessage();
                    CImage *image = CImage::create();
                    image->reset();
                    image->isEOS = TRUE;
                    if ((*m_ppRender) == NULL ||
                        !(*m_ppRender)->addImage(image)) {
                        CImage::release(image);
                    }
                    isComplete = true;
                }
                //播放结束，主动等待一段时间避免线程卡死
                usleep(5000);
            }
        }


        m_sllCurTS = 0;
        m_bIsStarted = FALSE;
        m_bIsStopped = FALSE;
        LOGI("video controller is stopped");
        return 0;
    }

    void CVController::start() {
        if (m_bIsStarted) {
            return;
        }
        m_cacheNumber = 0;
        m_sllCurTS = 0;

        if (m_ppRender != NULL && *m_ppRender != NULL)
            (*m_ppRender)->start();
        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL)
                m_vDec[i]->start();
        }
#if USING_VIDEO_DECODER_EX
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->start();
        }
#endif
        CController::start();
    }

    void CVController::stop() {
        if (m_ppRender != NULL && *m_ppRender != NULL)
            (*m_ppRender)->stop();
        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL)
                m_vDec[i]->stop();
        }
#if USING_VIDEO_DECODER_EX
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->stop();
        }
#endif
        CController::stop();
    }

    void CVController::pause() {
        CController::pause();
        if (m_ppRender != NULL && *m_ppRender != NULL)
            (*m_ppRender)->pause();
        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL)
                m_vDec[i]->pause();
        }

#if USING_VIDEO_DECODER_EX
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->pause();
        }
#endif
    }

    void CVController::resume() {
        CController::resume();
        if (m_ppRender != NULL && *m_ppRender != NULL)
            (*m_ppRender)->resume();
        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL)
                m_vDec[i]->resume();
        }
#if USING_VIDEO_DECODER_EX
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->resume();
        }
#endif
    }

    bool CVController::seekTo(s64 sllMicrosecond) {
#if USING_VIDEO_DECODER_EX
        CAutoLock autoLock(m_pLock);
        if (m_decoders.empty()) {
            return false;
        }
        if (m_ppRender != NULL && *m_ppRender != NULL) {
            (*m_ppRender)->flush();
        }
        TMessage *message = new TMessage;
        message->type = EM_CTRL_MESSAGE_SEEK;
        message->timestamp = sllMicrosecond;
        _seekList.push_back(message);
        return true;
#endif
    }

    bool CVController::locPreview(s64 sllMicrosecond) {
#if USING_VIDEO_DECODER_EX
        CAutoLock autoLock(m_pLock);
        if (m_decoders.empty()) {
            return false;
        }
        if (m_ppRender != NULL && *m_ppRender != NULL) {
            (*m_ppRender)->flush();
        }
        TMessage *message = new TMessage;
        message->type = EM_CTRL_MESSAGE_LOC_PREVIEW;
        message->timestamp = sllMicrosecond;
        _seekList.push_back(message);
        m_bIsPaused = FALSE;
        return true;
#endif
    }

    void CVController::_seekTo(int64_t sllMicrosecond) {
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->seekTo(sllMicrosecond);
        }
        m_sllCurTS = sllMicrosecond;
        m_cacheNumber = 0;
        if (m_ppRender != NULL && *m_ppRender != NULL) {
            (*m_ppRender)->flush();
            (*m_ppRender)->setCaching(true);
        }
        m_bIsPaused = FALSE;
    }

    BOOL32 CVController::init(CStoryboard **ppStoryboard, CRenderer **ppRenderer) {
        CAutoLock autoLock(m_pLock);
        if (ppStoryboard == NULL || *ppStoryboard == NULL || ppRenderer == NULL ||
            *ppRenderer == NULL) {
            return FALSE;
        }
        m_pTrackList = (*ppStoryboard)->getTrackByType(EM_TRACK_VIDEO);
        m_ppRender = ppRenderer;
        m_sllEndTS = (*ppStoryboard)->getDuration();
        return TRUE;
    }

    void CVController::prepare(BOOL32 isPlay, BOOL32 isWrite) {
        CAutoLock autoLock(m_pLock);
        if (m_pTrackList->size() == 0) {
            LOGE("there is no video track!");
            sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_VIDEO_CTRL_FAILED);
            return;
        }

        if (m_ppRender == NULL || *m_ppRender == NULL) {
            LOGE("there is no opengl player!");
            sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_VIDEO_CTRL_FAILED);
            return;
        }
        (*m_ppRender)->setCaching(isPlay ? true : false);

        //for (it = m_psTrack->begin(); it != m_psTrack->end();) {
        for (u32 i = 0; i < m_pTrackList->size(); ++i) {
            ITrack *pTrack = m_pTrackList->get(i);
            if (pTrack != NULL && pTrack->isSourceValid()) {
                CDec *dec = NULL;
                switch ((pTrack)->getSourceType()) {
                    case EM_SOURCE_MULTI: {
#if USING_VIDEO_DECODER_EX
                        IDecoder *pDecoder = new VideoMultiDecoder(pTrack);
                        if (pDecoder->prepare()) {
                            m_decoders.push_back(pDecoder);
                        } else {
                            delete pDecoder;
                        }
#else
                        dec = new CVMultiDec(pTrack);
                        if (dec && dec->prepare() == 0) {
                            m_vDec.push_back(dec);
                        } else {
                            LOGE("decoder of video track%d prepares failed!", (pTrack)->getId());
                            sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_VIDEO_DECODER_FAILED);
                        }
#endif
                    }
                        break;
                    case EM_SOURCE_FILE: {
#if USING_VIDEO_DECODER_EX
                        IDecoder *pDecoder = new VideoDecoder(pTrack);
                        if (pDecoder->prepare()) {
                            m_decoders.push_back(pDecoder);
                        } else {
                            delete pDecoder;
                        }
#else
                        dec = new CVDec(*it);
#endif
                    }
                        break;
                    case EM_SOURCE_BITMAP: {
#if USING_VIDEO_DECODER_EX
                        IDecoder *pDecoder = new BitmapDecoder(pTrack);
                        if (pDecoder->prepare()) {
                            m_decoders.push_back(pDecoder);
                        } else {
                            delete pDecoder;
                        }
#else
                        dec = new CVBitmapDec(*it);
#endif
                    }
                        break;
                    case EM_SOURCE_WEBP: {
#if USING_VIDEO_DECODER_EX
                        IDecoder *pDecoder = new WebpDecoder(pTrack);
                        if (pDecoder->prepare()) {
                            m_decoders.push_back(pDecoder);
                        } else {
                            delete pDecoder;
                        }
#else
                        dec = new CVWebPDec(*it);
#endif
                    }
                        break;
                    default:
                        LOGE("unknown source!");
                        break;
                }
            }
        }
        if (!(*m_ppRender)->prepare(isPlay ? true : false)) {
            LOGE("renderer prepares failed!");
        }
    }

    void CVController::sendImageDecodeCompleteMessage() {
        CAutoLock autoLock(m_pLock);
        if (m_cbDelegate != NULL && m_cbOnMessage != NULL) {
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_V_DECODE_COMPLETE, NULL);
        }
    }
}