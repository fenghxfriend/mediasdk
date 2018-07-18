//
// Created by ASUS on 2018/6/1.
//

#include <unistd.h>
#include <revcontroller.h>
#include <enum.h>
#include <sound.h>
#include <audiotrack.h>

namespace paomiantv {
    CRevController::CRevController() : m_ppDemuxer(NULL),
                                       m_ppRender(NULL),
                                       m_ppAudioTrack(NULL),
                                       m_pARevDec(NULL),
                                       m_pVRevDec(NULL),
                                       m_pVCutter(NULL),
                                       m_pACutter(NULL) {

    }

    CRevController::~CRevController() {
        stop();
        if (m_pARevDec != NULL) {
            delete m_pARevDec;
            m_pARevDec = NULL;
        }
        if (m_pVRevDec != NULL) {
            delete m_pVRevDec;
            m_pVRevDec = NULL;
        }
        if (m_pACutter != NULL) {
            delete m_pACutter;
            m_pACutter = NULL;
        }
        if (m_pVCutter != NULL) {
            delete m_pVCutter;
            m_pVCutter = NULL;
        }

    }

    long CRevController::run() {
        m_pThread->setName("CRevController");
        LOGI("reverse controller is started");
        m_bIsStarted = TRUE;
        bool isVComplete = false;
        bool isAComplete = false;
        (*m_ppDemuxer)->parse();
        while (!m_bIsStopped) {
            if (m_pVRevDec != NULL) {
                if (!isVComplete) {
                    CImage *image = CImage::create();
                    image->reset();
                    CVLayerParam *layer = NULL;
                    m_pVRevDec->getLayer(layer);
                    if (layer != NULL) {
                        image->m_sllTimeStampUS = layer->m_sllTimeStampUS;
                        image->m_vVLayerParam.push_back(layer);
                    } else {
                        isVComplete = true;
                        sendImageDecodeCompleteMessage();
                        image->isEOS = TRUE;

                    }
                    if ((*m_ppRender) == NULL ||
                        !(*m_ppRender)->addImage(image)) {
                        CImage::release(image);
                    }
                }
            }

            if (m_pARevDec != NULL) {
                if (!isAComplete) {

                    CSound *sound = CSound::create();
                    sound->reset();
                    sound->m_eSampleRate = m_ePlaySampleRate;
                    sound->m_byChannel = m_ePlaySampleRate;
                    sound->m_uFormat = m_ePlayFormat;

                    CAudio *audio = NULL;
                    m_pARevDec->getAudio(audio);
                    if (audio != NULL) {
                        sound->m_vAudios.push_back(audio);
                    } else {
                        isAComplete = true;
                        sendSoundDecodeCompleteMessage();
                        sound->isEOS = TRUE;
                    }

                    if ((*m_ppAudioTrack) == NULL ||
                        !(*m_ppAudioTrack)->writeSound(sound)) {
                        CSound::release(sound);
                    }

                }
            }
        }
        m_bIsStarted = FALSE;
        m_bIsStopped = FALSE;
        LOGI("reverse controller is stopped");
        return 0;
    }

    BOOL32
    CRevController::init(CDemuxer **ppDemuxer, CAudioTrack **ppAudioTrack, CRenderer **ppRender) {
        CAutoLock autoLock(m_pLock);
        if (ppDemuxer == NULL || *ppDemuxer == NULL ||
            ppAudioTrack == NULL || *ppAudioTrack == NULL ||
            ppRender == NULL || *ppRender == NULL) {
            LOGE("initialize controller failed, invalid parameters");
            return FALSE;
        }
        m_ppAudioTrack = ppAudioTrack;
        m_ppRender = ppRender;
        m_ppDemuxer = ppDemuxer;
        return TRUE;
    }

    void CRevController::prepare(CMuxer *muxer, s8 *pchTempPath, BOOL32 isVideoReverse,
                                 BOOL32 isAudioReverse) {
        CAutoLock autoLock(m_pLock);
        if (m_ppDemuxer == NULL || *m_ppDemuxer == NULL ||
            pchTempPath == NULL || strlen(pchTempPath) <= 0) {
            LOGE("prepare controller failed, demuxer is null");
            return;
        }
        if (isVideoReverse) {
            if (m_ppRender == NULL || *m_ppRender == NULL || !(*m_ppRender)->prepare()) {
                LOGE("renderer prepares failed!");
                sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_VIDEO_CTRL_FAILED);
                return;
            }
            m_pVRevDec = new VideoReverseDecoder(*m_ppDemuxer);
            if (m_pVRevDec && m_pVRevDec->prepare(pchTempPath) == 0) {
                if (m_cbDelegateEncode != NULL && m_cbOnStartVEncode) {
                    m_cbOnStartVEncode(m_cbDelegateEncode, VIDEO_MIME_TYPE, m_pVRevDec->getWidth(),
                                       m_pVRevDec->getHeight(), VIDEO_BIT_RATE, VIDEO_FRAME_RATE,
                                       VIDEO_I_FRAME_INTERVAL, VIDEO_AVC_PROFILE, VIDEO_AVC_LEVEL);
                }
            } else {
                LOGE("decoder of video prepares failed!");
                sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_VIDEO_DECODER_FAILED);
            }

        } else {
            m_pVCutter = new CCutter(muxer->getHandler());
            m_pVCutter->prepare((*m_ppDemuxer)->getSrc(), EM_TRACK_VIDEO, 0, INTEGER64_MAX_VALUE,
                                m_cbOnMessage, m_cbDelegate);
        }

        if (isAudioReverse) {
            if (m_ppAudioTrack == NULL || *m_ppAudioTrack == NULL) {
                LOGE("audio track is invalid!");
                sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_AUDIO_CTRL_FAILED);
                return;
            }


            m_pARevDec = new AudioReverseDecoder(*m_ppDemuxer);
            if (m_pARevDec && m_pARevDec->prepare(pchTempPath) == 0) {
            } else {
                LOGE("decoder of audio prepares failed!");
                sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_AUDIO_DECODER_FAILED);
            }
            u8 tracks = 1;
            s32 ids = 0;
            TAudioParams outParams;
            outParams.m_eSampleRate = m_ePlaySampleRate;
            outParams.m_ullChannelLayout = m_ullPlayChannelLayout;
            outParams.m_eFormat = m_ePlayFormat;

            TAudioParams inParams;

            if (!(*m_ppAudioTrack)->configure(tracks, &ids, &inParams, outParams, FALSE, TRUE)) {
                LOGE("audio track prepares failed!");
                sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_AUDIO_TRACK_FAILED);
            } else {
                if (m_cbDelegateEncode != NULL && m_cbOnStartAEncode) {
                    if (m_cbDelegateEncode != NULL && m_cbOnStartAEncode) {
                        m_cbOnStartAEncode(m_cbDelegateEncode, AUDIO_MIME_TYPE, m_ePlaySampleRate,
                                           av_get_channel_layout_nb_channels(
                                                   m_ullPlayChannelLayout), AUDIO_BIT_RATE,
                                           AUDIO_AAC_PROFILE);
                    }
                }
            }
        } else {
            m_pACutter = new CCutter(muxer->getHandler());
            m_pACutter->prepare((*m_ppDemuxer)->getSrc(), EM_TRACK_AUDIO, 0, INTEGER64_MAX_VALUE,
                                m_cbOnMessage, m_cbDelegate);
        }
    }

    void CRevController::start() {
        if (m_ppAudioTrack != NULL && *m_ppAudioTrack != NULL) {
            (*m_ppAudioTrack)->start();
        }
        if (m_ppRender != NULL && *m_ppRender != NULL) {
            (*m_ppRender)->start();
        }
        if (m_pVRevDec != NULL) {
            m_pVRevDec->start();
        }
        if (m_pARevDec != NULL) {
            m_pARevDec->start();
        }
        if (m_pVCutter != NULL) {
            m_pVCutter->start();
        }
        if (m_pACutter != NULL) {
            m_pACutter->start();
        }
        CController::start();
    }

    void CRevController::stop() {
        if (m_ppAudioTrack != NULL && *m_ppAudioTrack != NULL) {
            (*m_ppAudioTrack)->stop();
        }

        if (m_ppRender != NULL && *m_ppRender != NULL) {
            (*m_ppRender)->stop();
        }

        if (m_pVRevDec != NULL) {
            m_pVRevDec->stop();
        }
        if (m_pARevDec != NULL) {
            m_pARevDec->stop();
        }
        if (m_pVCutter != NULL) {
            m_pVCutter->stop();
        }
        if (m_pACutter != NULL) {
            m_pACutter->stop();
        }
        CController::stop();
    }

    void CRevController::resume() {
        CController::resume();
    }

    void CRevController::pause() {
        CController::pause();
    }

    bool CRevController::seekTo(s64 sllMicrosecond) {
        return false;
    }

    bool CRevController::locPreview(s64 sllMicrosecond) {
        return false;
    }

    void CRevController::sendImageDecodeCompleteMessage() {
        CAutoLock autoLock(m_pLock);
        if (m_cbDelegate != NULL && m_cbOnMessage != NULL) {
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_V_DECODE_COMPLETE, NULL);
        }
    }

    void CRevController::sendSoundDecodeCompleteMessage() {
        CAutoLock autoLock(m_pLock);
        if (m_cbDelegate != NULL && m_cbOnMessage != NULL) {
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_A_DECODE_COMPLETE, NULL);
        }
    }

    void CRevController::setOnEncodeThread(OnStartVEncodeCB cbOnStartVEncode,
                                           OnStartAEncodeCB cbOnStartAEncode, void *cbDelegate) {
        CAutoLock autoLock(m_pLock);
        m_cbOnStartVEncode = cbOnStartVEncode;
        m_cbOnStartAEncode = cbOnStartAEncode;
        m_cbDelegateEncode = cbDelegate;
    }
}
