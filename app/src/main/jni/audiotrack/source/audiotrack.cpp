//
// Created by ASUS on 2018/4/16.
//

#include <unistd.h>
#include "audiotrack.h"

namespace paomiantv {

    static u32 shiftchnlyt(const u64 outLayout) {
        u32 outSL = 0;
        switch (outLayout) {
            case AV_CH_LAYOUT_MONO:
                //mono
                outSL = SL_SPEAKER_FRONT_CENTER;
                break;
            case AV_CH_LAYOUT_STEREO:
                //stereo
                outSL = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
                break;
            case AV_CH_LAYOUT_2POINT1:
                //2.1
                outSL = SL_SPEAKER_LOW_FREQUENCY |
                        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
                break;
            case AV_CH_LAYOUT_3POINT1:
                //3.1
                outSL = SL_SPEAKER_LOW_FREQUENCY |
                        SL_SPEAKER_FRONT_CENTER |
                        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
                break;
            case AV_CH_LAYOUT_4POINT1:
                //4.1
                outSL = SL_SPEAKER_LOW_FREQUENCY |
                        SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT |
                        SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_BACK_CENTER;
                break;
            case AV_CH_LAYOUT_5POINT1:
                //5.1
                outSL = SL_SPEAKER_LOW_FREQUENCY |
                        SL_SPEAKER_FRONT_CENTER |
                        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT |
                        SL_SPEAKER_SIDE_LEFT | SL_SPEAKER_SIDE_RIGHT;
                break;
            case AV_CH_LAYOUT_6POINT1:
                //6.1
                outSL = SL_SPEAKER_LOW_FREQUENCY |
                        SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_BACK_CENTER |
                        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT |
                        SL_SPEAKER_SIDE_LEFT | SL_SPEAKER_SIDE_RIGHT;
                break;
            case AV_CH_LAYOUT_7POINT1:
                //7.1
                outSL = SL_SPEAKER_LOW_FREQUENCY |
                        SL_SPEAKER_FRONT_CENTER |
                        SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT |
                        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT |
                        SL_SPEAKER_SIDE_LEFT | SL_SPEAKER_SIDE_RIGHT;
                break;
            default:
                outSL = 0;
                break;
        }
        return outSL;
    }

    CAudioTrack::CAudioTrack() : m_isCaching(false),
                                 m_pMixSound(NULL),
                                 m_pBufferPlayer(NULL),
                                 m_pBufferWriter(NULL),
                                 m_puInputIds(NULL),
                                 m_ppbyInputBuffer(NULL),
                                 m_pnInputFrameSize(NULL),
                                 m_bIsStopped(FALSE),
                                 m_bIsPaused(FALSE),
                                 m_bIsStarted(FALSE),
                                 m_bIsPlay(FALSE),
                                 m_bIsWrite(FALSE),
                                 m_cbOnWrite(NULL),
                                 m_cbOnMessage(NULL),
                                 m_cbDelegate(NULL) {
        USE_LOG;
        m_pQueue = new CSafeQueue<CSound>(13);
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
        m_mapSourceIds.clear();
    }

    CAudioTrack::~CAudioTrack() {
        USE_LOG;
        stop();
        if (m_pMixSound != NULL) {
            m_pMixSound->destroy();
            m_pMixSound = NULL;
        }
        if (m_pBufferPlayer != NULL) {
            m_pBufferPlayer->destroy();
            m_pBufferPlayer = NULL;
        }
        if (m_pBufferWriter != NULL) {
            m_pBufferWriter->destroy();
            m_pBufferWriter = NULL;
        }
        if (m_pThread != NULL) {
            delete m_pThread;
            m_pThread = NULL;
        }
        if (m_pQueue != NULL) {
            delete m_pQueue;
            m_pQueue = NULL;
        }
        if (m_pLock != NULL) {
            m_pLock = new CLock;
            m_pLock = NULL;
        }
    }


    // byBitsPerSample have to times of 8
    BOOL32
    CAudioTrack::configure(const u8 inputs, const s32 *trackIds, TAudioParams *trackParams,
                           TAudioParams outParams, BOOL32 isPlay, BOOL32 isWrite) {
        USE_LOG;
        m_pQueue->clear();
        m_pQueue->enable();
        CAutoLock autoLock(m_pLock);
        m_bIsPlay = isPlay;
        m_bIsWrite = isWrite;
        m_bIsPaused = FALSE;
        m_bIsStarted = FALSE;
        m_bIsStopped = FALSE;
        int channels = av_get_channel_layout_nb_channels(outParams.m_ullChannelLayout);
        if ((outParams.m_eFormat < AV_SAMPLE_FMT_U8 || outParams.m_eFormat > AV_SAMPLE_FMT_S32)
            || outParams.m_eSampleRate == EM_SAMPLE_RATE_START
            || channels <= 0) {
            LOGE("we can not play the output data,params is incorrect");
            return FALSE;
        }

        u32 channelMask = shiftchnlyt(outParams.m_ullChannelLayout);
        if (channelMask == 0) {
            LOGE("shift the channel to channel mask failed!");
            return FALSE;
        };

        BOOL32 result = FALSE;
        do {
            // create audio buffer player with opensl
            m_pBufferPlayer = CBufferPlayer::Create(outParams.m_eSampleRate,
                                                    (u16) (0x0001
                                                            << outParams.m_eFormat),
                                                    (u32) channels,
                                                    channelMask);

            if (m_pBufferPlayer == NULL) {
                LOGE("create audio buffer player failed!")
                break;
            } else {
                m_pBufferPlayer->setOnMessageCB(m_cbOnMessage, m_cbDelegate);
            }

            // create audio buffer writer
            m_pBufferWriter = CBufferWriter::Create(outParams.m_eSampleRate,
                                                    (u16) (0x0001
                                                            << outParams.m_eFormat),
                                                    (u32) channels,
                                                    channelMask);

            if (m_pBufferWriter == NULL) {
                LOGE("create audio buffer writer failed!")
                break;
            } else {
                m_pBufferWriter->setOnWriteCB(m_cbOnMessage, m_cbOnWrite, m_cbDelegate);
            }

            // create audio complex filter
            m_pMixSound = CAFilterComplex::Create(outParams.m_eSampleRate, outParams.m_eFormat,
                                                  outParams.m_ullChannelLayout);
            if (m_pMixSound == NULL) {
                LOGE("create complex filter failed!")
                break;
            }
            bool added = true;
            u8 silence = 0;
            for (u8 i = 0; i < inputs; i++) {
                u32 id = 0;
                if (!m_pMixSound->addInput(trackParams[i].m_eSampleRate,
                                           trackParams[i].m_eFormat,
                                           trackParams[i].m_ullChannelLayout,
                                           trackParams[i].m_wWeigt,
                                           id)) {
                    LOGE("add input%d to complex filter failed!", i);
                    added = false;
                    break;
                }
                m_mapSourceIds[trackIds[i]] = id;
                if (trackParams[i].m_eSampleRate != outParams.m_eSampleRate ||
                    trackParams[i].m_eFormat != outParams.m_eFormat ||
                    trackParams[i].m_ullChannelLayout != outParams.m_ullChannelLayout) {
                    if (!m_pMixSound->addFormatInSource(id, outParams.m_eSampleRate,
                                                        outParams.m_eFormat,
                                                        outParams.m_ullChannelLayout)) {
                        LOGE("add resample filter to complex filter failed in input%d!", i);
                        added = false;
                        break;
                    }
                }
                if (trackParams[i].m_bIsSilence) {
                    silence++;
                }
            }
            if (!added) {
                LOGE("add inputs to complex filter failed!");
                break;
            }
            if (inputs > 0 && silence != inputs) {
                m_pMixSound->addVolume(inputs * 1.0f / (inputs - silence));
            }

            if (!m_pMixSound->configure()) {
                LOGE("configure complex filter failed!");
                break;
            }
            s32 tracks = m_pMixSound->getInputCount();
            m_puInputIds = (u32 *) malloc(sizeof(u32) * tracks);
            m_ppbyInputBuffer = (u8 **) malloc(sizeof(void *) * tracks);
            m_pnInputFrameSize = (int *) malloc(sizeof(int) * tracks);
            if (m_puInputIds == NULL || m_ppbyInputBuffer == NULL || m_pnInputFrameSize == NULL) {
                LOGE("no enough memory!");
                break;
            } else {
                memset(m_puInputIds, 0, sizeof(u32) * tracks);
                memset(m_ppbyInputBuffer, 0, sizeof(void *) * tracks);
                memset(m_pnInputFrameSize, 0, sizeof(int) * tracks);
            }
            result = TRUE;
        } while (FALSE);


        if (!result) {
            LOGE("configure audio track failed!")
            release();
        }
        return result;
    }

    // shut down the native audio system
    void CAudioTrack::release() {
        CAutoLock autoLock(m_pLock);
        if (m_pMixSound != NULL) {
            m_pMixSound->destroy();
            m_pMixSound = NULL;
        }
        if (m_pBufferPlayer != NULL) {
            m_pBufferPlayer->destroy();
            m_pBufferPlayer = NULL;
        }

        if (m_pBufferWriter != NULL) {
            m_pBufferWriter->destroy();
            m_pBufferWriter = NULL;
        }
        if (m_puInputIds != NULL) {
            free(m_puInputIds);
            m_puInputIds = NULL;
        }
        if (m_ppbyInputBuffer != NULL) {
            free(m_ppbyInputBuffer);
            m_ppbyInputBuffer = NULL;
        }
        if (m_pnInputFrameSize != NULL) {
            free(m_pnInputFrameSize);
            m_pnInputFrameSize = NULL;
        }
        m_mapSourceIds.clear();
    }

    void CAudioTrack::start() {
        LOGI("CAudioTrack::startThread");
        if (m_pBufferPlayer != NULL) {
            m_pBufferPlayer->start();
        }
        m_pQueue->enable();
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted) {
            return;
        }
        if (!m_pThread->start()) {
            LOGE("start audio track thread failed!");
            return;
        }
    }

    void CAudioTrack::stop() {
        LOGI("CAudioTrack::stopThread");
        if (m_pBufferPlayer != NULL) {
            m_pBufferPlayer->stop();
        }
        m_pQueue->disable();
        m_pLock->lock();
        if (m_bIsStarted && !m_bIsStopped) {
            m_bIsStopped = TRUE;
            m_pLock->acttive();
        }
        m_pLock->unlock();
        m_pThread->join();
        m_pQueue->clear();
    }

    void CAudioTrack::flush() {
        CAutoLock autoLock(m_pLock);
        if (m_pQueue) {
            m_pQueue->disable();
            m_pQueue->clear();
            m_pQueue->enable();
        }
        if (m_pBufferPlayer) {
            m_pBufferPlayer->flush();
        }
    }

    void CAudioTrack::pause() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && !m_bIsPaused) {
            if (m_pBufferPlayer != NULL) {
                m_pBufferPlayer->pause();
            }
            m_bIsPaused = TRUE;
        }
    }

    void CAudioTrack::resume() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && m_bIsPaused) {
            if (m_pBufferPlayer != NULL) {
                m_pBufferPlayer->resume();
            }
            m_bIsPaused = FALSE;
            m_pLock->acttive();
        }
    }

    BOOL32 CAudioTrack::writeSound(CSound *pSound) {
        if (m_pQueue != NULL) {
//            LOGE("frame is invalid or queue is null or renderer stopped! ");
            return m_pQueue->push(pSound);
        }
        return FALSE;
    }

    void *CAudioTrack::ThreadWrapper(void *pThis) {
        CAudioTrack *p = (CAudioTrack *) pThis;
        return p->run();
    }

    void *CAudioTrack::run() {
        USE_LOG;
        m_pThread->setName("CAudioTrack");
        m_pLock->lock();
        LOGI("audiotrack thread is started");
        m_bIsStarted = TRUE;

        while (!m_bIsStopped) {
            while (!m_bIsStopped && m_bIsPaused) {
                m_pLock->wait(1000);
            }
            if (!m_bIsStopped) {
                m_pLock->unlock();
                if (process()) {
                }
                m_pLock->lock();
            }
        }
        m_bIsStarted = FALSE;
        m_bIsStopped = FALSE;
        LOGI("audiotrack thread is stopped");
        m_pLock->unlock();
        return 0;
    }

    BOOL32 CAudioTrack::process() {
        if (m_isCaching) {
            //正在缓存数据，主动等待一段时间以避免卡死线程
            usleep(3000);
            return FALSE;
        }

        CSound *pSound = NULL;
        m_pQueue->pop(pSound);
        if (pSound == NULL || pSound->m_vAudios.size() == 0) {

            if (pSound != NULL && pSound->isEOS) {
                LOGE("Audio stream is EOS! ");
                if (m_bIsPlay && m_pBufferPlayer != NULL) {
                    m_pBufferPlayer->sendBuffer(NULL, 0);
                }
                if (m_bIsWrite && m_pBufferWriter != NULL) {
                    m_pBufferWriter->writeBuffer(NULL, 0);
                }
                CSound::release(pSound);
                return FALSE;
            }
            LOGE("frame is invalid! ");
            if (pSound != NULL) {
                CSound::release(pSound);
            }
            return FALSE;
        }
        BOOL32 re = processSound(pSound);
        pSound->reset();
        CSound::release(pSound);
        return re;
    }


    // all audio track data processed when decoded! We only mix the layers!
    BOOL32 CAudioTrack::processSound(const CSound *pSound) {
        // process audio
        if (m_pMixSound == NULL || m_puInputIds == NULL || m_ppbyInputBuffer == NULL ||
            m_pnInputFrameSize == NULL) {
            return FALSE;
        }
        s32 slayerSize = pSound->m_vAudios.size();
        s32 tracks = m_pMixSound->getInputCount();
        if (slayerSize <= tracks) {
            memset(m_puInputIds, 0, sizeof(u32) * tracks);
            memset(m_ppbyInputBuffer, 0, sizeof(void *) * tracks);
            memset(m_pnInputFrameSize, 0, sizeof(int) * tracks);
            int index = 0;
            for (s32 i = 0; i < tracks && i < slayerSize; i++) {
                int channels = 0;
                if (pSound->m_vAudios[i] != NULL &&
                    pSound->m_vAudios[i]->m_pbyVoice != NULL &&
                    pSound->m_vAudios[i]->m_uSize > 0 &&
                    pSound->m_vAudios[i]->m_tParams.m_eSampleRate > EM_SAMPLE_RATE_START &&
                    pSound->m_vAudios[i]->m_tParams.m_eFormat > AV_SAMPLE_FMT_NONE &&
                    (channels = av_get_channel_layout_nb_channels(
                            pSound->m_vAudios[i]->m_tParams.m_ullChannelLayout)) > 0 &&
                    (pSound->m_vAudios[i]->m_uSize / channels /
                     (0x0001 << pSound->m_vAudios[i]->m_tParams.m_eFormat)) ==
                    AUDIO_SAMPLE_COUNT_PER_FRAME) {
                    m_puInputIds[index] = m_mapSourceIds[pSound->m_vAudios[i]->m_uTrackId];
                    m_ppbyInputBuffer[index] = pSound->m_vAudios[i]->m_pbyVoice +
                                               pSound->m_vAudios[i]->m_uOffset;
                    m_pnInputFrameSize[index] = AUDIO_SAMPLE_COUNT_PER_FRAME;
                    index++;
                } else {
                    LOGW("the track%u buffer is invalid!",
                         pSound->m_vAudios[i]->m_uTrackId);
                }
            }
            u8 *out = NULL;
            int outSize = 0;
            m_pMixSound->process(index, m_puInputIds, m_ppbyInputBuffer, m_pnInputFrameSize, out,
                                 outSize);
            if (m_bIsPlay) {
                m_pBufferPlayer->sendBuffer(out, outSize);
            }
            if (m_bIsWrite) {
                m_pBufferWriter->writeBuffer(out, outSize);
            }

        }
        return TRUE;
    }

    void
    CAudioTrack::setOnMessageCB(OnMessageCB cbOnMessage, OnWritePCMCB cbOnWrite, void *cbDelegate) {
        CAutoLock autoLock(m_pLock);
        m_cbOnMessage = cbOnMessage;
        m_cbDelegate = cbDelegate;
        m_cbOnWrite = cbOnWrite;
        if (m_pBufferPlayer != NULL) {
            m_pBufferPlayer->setOnMessageCB(m_cbOnMessage, m_cbDelegate);
        }
        if (m_pBufferWriter != NULL) {
            m_pBufferWriter->setOnWriteCB(m_cbOnMessage, m_cbOnWrite, m_cbDelegate);
        }
    }
} // namespace paomiantv