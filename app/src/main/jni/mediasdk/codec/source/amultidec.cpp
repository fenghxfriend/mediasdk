//
// Created by ASUS on 2018/4/20.
//

#include <multitrack.h>
#include <track.h>
#include <amultidec.h>
#include <silencedec.h>

namespace paomiantv {
    CAMultiDec::CAMultiDec(ITrack *const &pTrack) : CADec(pTrack), m_nIndex(0) {
        USE_LOG;
    }

    CAMultiDec::~CAMultiDec() {
        USE_LOG;
        stop();
        release();
        int size = m_vADecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vADecs[i] != NULL) {
                delete m_vADecs[i];
            }
        }
        m_vADecs.clear();
    }

    int CAMultiDec::prepare() {
        if (m_pTrack != NULL && m_pTrack->getSourceType() == EM_SOURCE_MULTI) {
            CMultiTrack *pMultiTrack = (CMultiTrack *) m_pTrack;
            int size = pMultiTrack->getTrackCount();
            for (u32 i = 0; i < size; i++) {
                ITrack *t = NULL;
                if (pMultiTrack->getTrack(t, i)) {
                    CDec *dec = NULL;
                    switch (t->getSourceType()) {
                        case EM_SOURCE_MULTI: {
                            dec = new CAMultiDec(t);
                        }
                            break;
                        case EM_SOURCE_FILE: {
                            dec = new CADec(t);
                        }
                            break;
                        case EM_SOURCE_SILENCE: {
                            dec = new CASilenceDec(t);
                        }
                            break;
                        default:
                            LOGE("unknown source!");
                            break;
                    }
                    if (dec && dec->prepare() == 0) {
                        m_vADecs.push_back(dec);
                    } else {
                        LOGE("the decoder of track%d in multi track prepares failed!", i);
                        return -1;
                    }

                } else {
                    LOGE("get track from multi track failed!");
                    return -1;
                }
            }
            m_sllStartPlayUS = m_pTrack->getPlayStart() * 1000;
            m_sllEndPlayUS = m_pTrack->getPlayDuration() < 0 ? -1 :
                             (m_sllStartPlayUS + m_pTrack->getPlayDuration()) * 1000;
            m_uOneFrameSize = (u32) (AUDIO_SAMPLE_COUNT_PER_FRAME *
                                     av_get_channel_layout_nb_channels(
                                             m_ullChannelLayout) *
                                     (0x0001 << m_eFormat));
            m_pbyRemainderBuffer = (u8 *) malloc(m_uOneFrameSize);
        } else {
            LOGE("the multi track is invalid!");
            return -1;
        }
        return 0;
    }

    void CAMultiDec::release() {
        int size = m_vADecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vADecs[i] != NULL) {
                m_vADecs[i]->release();
            }
        }
    }

    BOOL32 CAMultiDec::getAudio(CAudio *&pAudio) {
        pAudio = NULL;
        if (!m_bIsFinished && m_sllCurrPlayUS >= m_sllStartPlayUS &&
            (m_sllEndPlayUS < 0 ? TRUE : m_sllCurrPlayUS <= m_sllEndPlayUS)) {
            bool got_frame = false;
            while (!got_frame && !m_bIsFinished) {
                CAudio *tmp = NULL;
                m_vADecs[m_nIndex]->getAudio(tmp);
                if (tmp == NULL) {
                    u8 *buffer = NULL;
                    u32 size = 0;
                    m_vADecs[m_nIndex]->getRemainderBuffer(buffer, size);
                    if (buffer != NULL && size > 0) {
                        if (m_uRemainderBufferSize + size >= m_uOneFrameSize) {
                            tmp = CAudio::create();
                            tmp->resize(m_uOneFrameSize);
                            tmp->m_uSize = m_uOneFrameSize;
                            tmp->m_uTrackId = m_pTrack->getId();
                            memcpy(tmp->m_pbyVoice, m_pbyRemainderBuffer,
                                   m_uRemainderBufferSize);
                            memcpy(tmp->m_pbyVoice + m_uRemainderBufferSize,
                                   buffer,
                                   m_uOneFrameSize - m_uRemainderBufferSize);

                            tmp->m_tParams.m_eFormat = m_eFormat;
                            tmp->m_tParams.m_ullChannelLayout = m_ullChannelLayout;
                            tmp->m_tParams.m_eSampleRate = m_eSampleRate;

                            u32 remainder =
                                    size + m_uRemainderBufferSize - m_uOneFrameSize;
                            memcpy(m_pbyRemainderBuffer,
                                   buffer + (m_uOneFrameSize - m_uRemainderBufferSize),
                                   remainder);
                            m_uRemainderBufferSize = remainder;
                        } else {
                            memcpy(m_pbyRemainderBuffer + m_uRemainderBufferSize, buffer, size);
                            m_uRemainderBufferSize += size;
                        }

                    }
                    if (m_nIndex == m_vADecs.size() - 1) {
                        if (tmp == NULL) {
                            if (m_uRemainderBufferSize > 0) {
                                tmp = CAudio::create();
                                tmp->resize(m_uOneFrameSize);
                                tmp->m_uSize = m_uOneFrameSize;
                                tmp->m_uTrackId = m_pTrack->getId();
                                memcpy(tmp->m_pbyVoice, m_pbyRemainderBuffer,
                                       m_uRemainderBufferSize);

                                tmp->m_tParams.m_eFormat = m_eFormat;
                                tmp->m_tParams.m_ullChannelLayout = m_ullChannelLayout;
                                tmp->m_tParams.m_eSampleRate = m_eSampleRate;

                                m_uRemainderBufferSize = 0;
                            } else {
                                LOGE("Input finished. Write NULL frame \n");
                                m_bIsFinished = TRUE;
                            }
                        }
                    } else {
                        LOGE("Child Audio Track%u has finished, go to next!!!!!!\n",
                             m_vADecs[m_nIndex]->getTrackId());
                        m_nIndex++;
                    }
                    if (tmp != NULL) {
                        pAudio = tmp;
                        got_frame = true;
                    }
                } else {
                    if (m_uRemainderBufferSize > 0) {
                        CAudio *audio = CAudio::create();
                        audio->resize(m_uOneFrameSize);
                        audio->m_uSize = m_uOneFrameSize;
                        audio->m_uTrackId = m_pTrack->getId();
                        memcpy(audio->m_pbyVoice, m_pbyRemainderBuffer,
                               m_uRemainderBufferSize);
                        memcpy(audio->m_pbyVoice + m_uRemainderBufferSize,
                               tmp->m_pbyVoice,
                               m_uOneFrameSize - m_uRemainderBufferSize);

                        audio->m_tParams.m_eFormat = m_eFormat;
                        audio->m_tParams.m_ullChannelLayout = m_ullChannelLayout;
                        audio->m_tParams.m_eSampleRate = m_eSampleRate;

                        u32 remainder =
                                tmp->m_uSize + m_uRemainderBufferSize - m_uOneFrameSize;
                        memcpy(m_pbyRemainderBuffer,
                               tmp->m_pbyVoice +
                               (m_uOneFrameSize - m_uRemainderBufferSize),
                               remainder);
                        m_uRemainderBufferSize = remainder;
                        CAudio::release(tmp);
                        pAudio = audio;
                    } else {
                        pAudio = tmp;
                        pAudio->m_uTrackId = m_pTrack->getId();
                    }
                    got_frame = true;
                }
            }
        }

        m_sllCurrPlayUS += (AUDIO_SAMPLE_COUNT_PER_FRAME * 1000 * 1000 / m_eSampleRate);
        return TRUE;
    }

    void CAMultiDec::start() {
        int size = m_vADecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vADecs[i] != NULL) {
                m_vADecs[i]->start();
            }
        }
    }

    void CAMultiDec::stop() {
        int size = m_vADecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vADecs[i] != NULL) {
                m_vADecs[i]->stop();
            }
        }
    }

    void CAMultiDec::pause() {
        int size = m_vADecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vADecs[i] != NULL) {
                m_vADecs[i]->pause();
            }
        }
    }

    void CAMultiDec::resume() {
        int size = m_vADecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vADecs[i] != NULL) {
                m_vADecs[i]->resume();
            }
        }
    }
}