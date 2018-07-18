//
// Created by ASUS on 2018/5/16.
//

#include "silencedec.h"

namespace paomiantv {
    CASilenceDec::CASilenceDec(ITrack *const &pTrack) : CDec(pTrack),
                                                        m_sllCurrPlayUS(0),
                                                        m_sllStartPlayUS(0),
                                                        m_sllEndCutUS(0),
                                                        m_sllLastPTSUS(0), m_uPerFrameSize(0) {

    }

    CASilenceDec::~CASilenceDec() {
    }

    int CASilenceDec::prepare() {
        if (m_pTrack != NULL &&
            m_pTrack->getType() == EM_TRACK_AUDIO &&
            m_pTrack->getSourceType() == EM_SOURCE_SILENCE) {
            m_uPerFrameSize = AUDIO_SAMPLE_COUNT_PER_FRAME * (0x0001) << AV_SAMPLE_FMT_S16;
            m_uPerFrameSize *= (uint32_t) av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            m_sllStartPlayUS = m_pTrack->getPlayStart() * 1000;
            m_sllEndPlayUS = m_pTrack->getPlayDuration() < 0 ? -1 :
                             (m_sllStartPlayUS + m_pTrack->getPlayDuration()) * 1000;
            if (!m_pTrack->isIndependent()) {
                m_sllCurrPlayUS = m_sllStartPlayUS;
                if (m_sllEndPlayUS < 0) {
                    m_sllEndPlayUS = m_sllStartPlayUS + ((ITrack *) m_pTrack)->getDataDuration();
                }
            }
        } else {
            LOGE("the silence track is invalid!");
            return -1;
        }
        return 0;
    }

    void CASilenceDec::release() {

    }

    void CASilenceDec::start() {
        LOGI("CASilenceDec::startThread");
    }

    void CASilenceDec::stop() {
        LOGI("CASilenceDec::stopThread");
    }

    void CASilenceDec::pause() {

    }

    void CASilenceDec::resume() {

    }

    BOOL32 CASilenceDec::getLayer(CVLayerParam *&pLayer) {

        pLayer = NULL;
        return FALSE;
    }

    BOOL32 CASilenceDec::getAudio(CAudio *&pAudio) {
        pAudio = NULL;
        if (m_sllCurrPlayUS >= m_sllStartPlayUS &&
            (m_sllEndPlayUS < 0 ? TRUE : m_sllCurrPlayUS <= m_sllEndPlayUS)) {
            pAudio = CAudio::create();
            pAudio->resize(m_uPerFrameSize);
            pAudio->m_uSize = m_uPerFrameSize;
            pAudio->m_uTrackId = m_pTrack->getId();
            pAudio->m_tParams.m_eFormat = AV_SAMPLE_FMT_S16;
            pAudio->m_tParams.m_ullChannelLayout = AV_CH_LAYOUT_STEREO;
            pAudio->m_tParams.m_eSampleRate = EM_SAMPLE_RATE_44_1;
        }
        m_sllCurrPlayUS += (AUDIO_SAMPLE_COUNT_PER_FRAME * 1000 * 1000 / EM_SAMPLE_RATE_44_1);
        return FALSE;
    }

    BOOL32 CASilenceDec::getRemainderBuffer(u8 *&out, u32 &size) {
        out = NULL;
        size = 0;
        return FALSE;
    }

}