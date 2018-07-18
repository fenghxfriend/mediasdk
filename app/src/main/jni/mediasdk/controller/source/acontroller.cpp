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

#include <acontroller.h>
#include <amultidec.h>
#include <sound.h>
#include <afilter.h>
#include <multitrack.h>
#include <unistd.h>
#include <AudioDecoder.h>
#include <AudioMultiDecoder.h>
#include <SilenceDecoder.h>

namespace paomiantv {
    CAController::CAController() : m_ppAudioTrack(NULL) {
        USE_LOG;
        m_vDec.clear();
#if USING_AUDIO_DECODER_EX
        m_decoders.clear();
#endif
        _seekList.clear();
    }

    CAController::~CAController() {
        USE_LOG;
        stop();
        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL) {
                delete m_vDec[i];
            }
        }
        m_vDec.clear();
#if USING_AUDIO_DECODER_EX
        std::vector<IDecoder *> deleteList;
        std::swap(m_decoders, deleteList);
        for (auto pDecoder : deleteList) {
            delete pDecoder;
        }
        deleteList.clear();
#endif
    }

    long CAController::run() {
        m_pThread->setName("CAController");
        bool isComplete = false;
        LOGI("audio controller is started");
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

                CSound *sound = CSound::create();
                sound->reset();
                sound->m_eSampleRate = m_ePlaySampleRate;
                sound->m_byChannel = m_ePlaySampleRate;
                sound->m_uFormat = m_ePlayFormat;
                for (int i = 0; i < m_vDec.size(); i++) {
                    CAudio *audio = NULL;
                    m_vDec[i]->getAudio(audio);
                    if (audio != NULL) {
                        sound->m_vAudios.push_back(audio);
                    }
                }
#if USING_AUDIO_DECODER_EX
                for (size_t i = 0; i < m_decoders.size(); ++i) {
                    CAudio *pAudio = m_decoders[i]->getAudio(m_sllCurTS);
                    if (pAudio) {
                        sound->m_vAudios.push_back(pAudio);
                    }
                }
#endif
                sound->m_sllTimeStampUS = m_sllCurTS;
                m_sllCurTS += AUDIO_SAMPLE_COUNT_PER_FRAME * 1000 * 1000 / m_ePlaySampleRate;

                if (msgtype == EM_CTRL_MESSAGE_LOC_PREVIEW ||
                    sound->m_vAudios.size() <= 0 ||
                    (*m_ppAudioTrack) == NULL ||
                    !(*m_ppAudioTrack)->writeSound(sound)) {
                    CSound::release(sound);
                }

                if (msgtype == EM_CTRL_MESSAGE_LOC_PREVIEW) {
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
                    sendSoundDecodeCompleteMessage();
                    CSound *sound = CSound::create();
                    sound->reset();
                    sound->isEOS = TRUE;
                    if ((*m_ppAudioTrack) == NULL ||
                        !(*m_ppAudioTrack)->writeSound(sound)) {
                        CSound::release(sound);
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
        LOGI("audio controller is stopped");
        return 0;
    }

    void CAController::start() {
        if (m_bIsStarted) {
            return;
        }
        m_cacheNumber = 0;
        m_sllCurTS = 0;

        if (m_ppAudioTrack != NULL && *m_ppAudioTrack != NULL)
            (*m_ppAudioTrack)->start();

        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL)
                m_vDec[i]->start();
        }
#if USING_AUDIO_DECODER_EX
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->start();
        }
#endif
        CController::start();
    }

    void CAController::stop() {
        if (m_ppAudioTrack != NULL && *m_ppAudioTrack != NULL)
            (*m_ppAudioTrack)->stop();
        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL)
                m_vDec[i]->stop();
        }
#if USING_AUDIO_DECODER_EX
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->stop();
        }
#endif
        CController::stop();
    }

    void CAController::pause() {
        CController::pause();
        if (m_ppAudioTrack != NULL && *m_ppAudioTrack != NULL)
            (*m_ppAudioTrack)->pause();

        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL)
                m_vDec[i]->pause();
        }
#if USING_AUDIO_DECODER_EX
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->pause();
        }
#endif
    }

    void CAController::resume() {
        CController::resume();
        if (m_ppAudioTrack != NULL && *m_ppAudioTrack != NULL)
            (*m_ppAudioTrack)->resume();

        for (int i = 0; i < m_vDec.size(); i++) {
            if (m_vDec[i] != NULL)
                m_vDec[i]->resume();
        }
#if USING_AUDIO_DECODER_EX
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->resume();
        }
#endif
    }

    bool CAController::seekTo(s64 sllMicrosecond) {
#if USING_AUDIO_DECODER_EX
        CAutoLock autoLock(m_pLock);
        if (m_decoders.empty()) {
            return false;
        }

        if (m_ppAudioTrack && *m_ppAudioTrack) {
            (*m_ppAudioTrack)->flush();
        }
        TMessage *message = new TMessage;
        message->type = EM_CTRL_MESSAGE_SEEK;
        message->timestamp = sllMicrosecond;
        _seekList.push_back(message);

        return true;
#endif
    }

    bool CAController::locPreview(s64 sllMicrosecond) {
#if USING_AUDIO_DECODER_EX
        CAutoLock autoLock(m_pLock);
        if (m_decoders.empty()) {
            return false;
        }
        if (m_ppAudioTrack && *m_ppAudioTrack) {
            (*m_ppAudioTrack)->flush();
        }
        TMessage *message = new TMessage;
        message->type = EM_CTRL_MESSAGE_LOC_PREVIEW;
        message->timestamp = sllMicrosecond;
        _seekList.push_back(message);
        m_bIsPaused = FALSE;
        return true;
#endif
    }

    void CAController::_seekTo(int64_t sllMicrosecond) {
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            m_decoders[i]->seekTo(sllMicrosecond);
        }
        m_sllCurTS = sllMicrosecond;
        m_cacheNumber = 0;
        if (m_ppAudioTrack && *m_ppAudioTrack) {
            (*m_ppAudioTrack)->setCaching(true);
            (*m_ppAudioTrack)->flush();
        }
        m_bIsPaused = FALSE;
    }

    BOOL32 CAController::init(CStoryboard **ppStoryboard, CAudioTrack **ppAudiotrack) {
        CAutoLock autoLock(m_pLock);
        if (ppStoryboard == NULL || *ppStoryboard == NULL || ppAudiotrack == NULL ||
            *ppAudiotrack == NULL) {
            return FALSE;
        }
        m_pTrackList = (*ppStoryboard)->getTrackByType(EM_TRACK_AUDIO);
        m_ppAudioTrack = ppAudiotrack;
        m_sllEndTS = (*ppStoryboard)->getDuration();
        return TRUE;
    }

    void CAController::prepare(BOOL32 isPlay, BOOL32 isWrite) {
        CAutoLock autoLock(m_pLock);
        if (m_pTrackList->size() == 0) {
            LOGE("there is no audio track!");
            sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_AUDIO_CTRL_FAILED);
            return;
        }
        if (m_ppAudioTrack == NULL || *m_ppAudioTrack == NULL) {
            LOGE("there is no opensl player!");
            sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_AUDIO_CTRL_FAILED);
            return;
        }
        (*m_ppAudioTrack)->setCaching(isPlay ? true : false);

        for (u32 i = 0; i < m_pTrackList->size(); ++i) {
            ITrack *pTrack = m_pTrackList->get(i);
            if (pTrack != NULL && pTrack->isSourceValid()) {
                CDec *dec = NULL;
                switch ((pTrack)->getSourceType()) {
                    case EM_SOURCE_MULTI: {
#if USING_AUDIO_DECODER_EX
                        IDecoder *pDecoder = new AudioMultiDecoder(pTrack);
                        if (pDecoder->prepare()) {
                            m_decoders.push_back(pDecoder);
                        } else {
                            delete pDecoder;
                        }
#else
                        dec = new CAMultiDec(pTrack);
                        if (dec && dec->prepare() == 0) {
                            m_vDec.push_back(dec);
                        } else {
                            LOGE("decoder of audio track%d prepares failed!", (pTrack)->getId());
                            sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_AUDIO_DECODER_FAILED);
                        }
#endif
                    }
                        break;
                    case EM_SOURCE_FILE: {
#if USING_AUDIO_DECODER_EX
                        IDecoder *pDecoder = new AudioDecoder(pTrack);
                        if (pDecoder->prepare()) {
                            m_decoders.push_back(pDecoder);
                        } else {
                            delete pDecoder;
                        }
#else
                        dec = new CADec(*it);
#endif
                    }
                        break;
                    case EM_SOURCE_SILENCE: {
#if USING_AUDIO_DECODER_EX
                        IDecoder *pDecoder = new SilenceDecoder(pTrack);
                        if (pDecoder->prepare()) {
                            m_decoders.push_back(pDecoder);
                        } else {
                            delete pDecoder;
                        }
#else
                        dec = new CASilenceDec(*it);
#endif
                    }
                        break;
                    default:
                        LOGE("unknown source!");
                        break;
                }
            }
        }

#if USING_AUDIO_DECODER_EX
        u8 tracks = (u8) (m_vDec.size() + m_decoders.size());
        TAudioParams outParams;
        outParams.m_eSampleRate = m_ePlaySampleRate;
        outParams.m_ullChannelLayout = m_ullPlayChannelLayout;
        outParams.m_eFormat = m_ePlayFormat;
        TAudioParams *inParams = new TAudioParams[tracks];
        s32 *ids = new s32[tracks];
        size_t offset = 0;
        for (size_t i = 0; i < m_vDec.size(); ++i) {
            inParams[offset].m_eFormat = ((CADec *) m_vDec[i])->getFormat();
            inParams[offset].m_ullChannelLayout = ((CADec *) m_vDec[i])->getChannelLayout();
            inParams[offset].m_eSampleRate = ((CADec *) m_vDec[i])->getSampleRate();
            inParams[offset].m_wWeigt = ((CADec *) m_vDec[i])->getWeight();
            inParams[offset].m_bIsSilence = m_vDec[i]->getSourceType() == EM_SOURCE_SILENCE;
            ids[offset] = m_vDec[i]->getTrackId();
            ++offset;
        }
        for (size_t i = 0; i < m_decoders.size(); ++i) {
            inParams[offset].m_eFormat = AudioDecoder::s_sampleFormat;
            inParams[offset].m_ullChannelLayout = AudioDecoder::s_channelLayout;
            inParams[offset].m_eSampleRate = AudioDecoder::s_sampleRate;
            inParams[offset].m_wWeigt = m_decoders[i]->getWeight();
            inParams[offset].m_bIsSilence = m_decoders[i]->getSourceType() == EM_SOURCE_SILENCE;
            ids[offset] = m_decoders[i]->getTrackID();
            ++offset;
        }
        if (!(*m_ppAudioTrack)->configure(tracks, ids, inParams, outParams, isPlay, isWrite)) {
            LOGE("audio track prepares failed!");
            sendErrorMessage(EM_MESSAGE_ID_ERROR_PREPARE_AUDIO_TRACK_FAILED);
        }
        delete[]inParams;
        delete[]ids;
#else
        u8 tracks = static_cast<u8>(m_vDec.size());
        TAudioParams outParams;
        outParams.m_eSampleRate = m_ePlaySampleRate;
        outParams.m_ullChannelLayout = m_ullPlayChannelLayout;
        outParams.m_eFormat = m_ePlayFormat;
        TAudioParams *inParams = new TAudioParams[tracks];
        s32 *ids = new s32[tracks];
        for (int i = 0; i < m_vDec.size(); i++) {
            inParams[i].m_eFormat = ((CADec *) m_vDec[i])->getFormat();
            inParams[i].m_ullChannelLayout = ((CADec *) m_vDec[i])->getChannelLayout();
            inParams[i].m_eSampleRate = ((CADec *) m_vDec[i])->getSampleRate();
            inParams[offset].m_wWeigt = ((CADec *) m_vDec[i])->getWeight();
            ids[i] = m_vDec[i]->getTrackId();
        }
        if (!(*m_ppAudioTrack)->configure(tracks, ids, inParams, outParams, isPlay, isWrite)) {
            LOGE("audio track prepares failed!");
        }
        delete[]inParams;
        delete[]ids;
#endif
    }

    void CAController::sendSoundDecodeCompleteMessage() {
        CAutoLock autoLock(m_pLock);
        if (m_cbDelegate != NULL && m_cbOnMessage != NULL) {
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_A_DECODE_COMPLETE, NULL);
        }
    }
}