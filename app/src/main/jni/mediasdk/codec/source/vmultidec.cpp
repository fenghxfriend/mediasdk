//
// Created by ASUS on 2018/4/20.
//


#include <multitrack.h>
#include <wpdec.h>
#include <bitmapdec.h>
#include "../include/vmultidec.h"

namespace paomiantv {
    CVMultiDec::CVMultiDec(ITrack *const &pTrack) : CVDec(pTrack), m_nIndex(0) {
        USE_LOG;

    }

    CVMultiDec::~CVMultiDec() {
        USE_LOG;
        stop();
        release();
        int size = m_vVDecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vVDecs[i] != NULL) {
                delete m_vVDecs[i];
            }
        }
        m_vVDecs.clear();
    }

    int CVMultiDec::prepare() {
        if (m_pTrack != NULL && m_pTrack->getSourceType() == EM_SOURCE_MULTI) {
            CMultiTrack *pMultiTrack = (CMultiTrack *) m_pTrack;
            int size = pMultiTrack->getTrackCount();
            for (u32 i = 0; i < size; i++) {
                ITrack *t = NULL;
                uint8_t *data = nullptr;
                uint32_t size = 0;
                if (pMultiTrack->getTrack(t, i) &&
                    t != NULL &&
                    t->getType() == EM_TRACK_VIDEO &&
                    (t->getSourceData(data, size), (data != NULL && size != 0))) {
                    CDec *dec = NULL;
                    switch (t->getSourceType()) {
                        case EM_SOURCE_MULTI: {
                            dec = new CVMultiDec(t);
                        }
                            break;
                        case EM_SOURCE_FILE: {
                            dec = new CVDec(t);
                        }
                            break;
                        case EM_SOURCE_BITMAP: {
                            dec = new CVBitmapDec(t);
                        }
                            break;
                        case EM_SOURCE_WEBP: {
                            dec = new CVWebPDec(t);
                        }
                            break;
                        default:
                            LOGE("unknown source!");
                            break;
                    }
                    if (dec && dec->prepare() == 0) {
                        m_vVDecs.push_back(dec);
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
        } else {
            LOGE("the multi track is invalid!");
            return -1;
        }
        return 0;
    }

    void CVMultiDec::release() {
        int size = m_vVDecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vVDecs[i] != NULL) {
                m_vVDecs[i]->release();
            }
        }
    }

    void CVMultiDec::start() {
        int size = m_vVDecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vVDecs[i] != NULL) {
                m_vVDecs[i]->start();
            }
        }
    }

    void CVMultiDec::stop() {
        int size = m_vVDecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vVDecs[i] != NULL) {
                m_vVDecs[i]->stop();
            }
        }
    }

    void CVMultiDec::pause() {
        int size = m_vVDecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vVDecs[i] != NULL) {
                m_vVDecs[i]->pause();
            }
        }
    }

    void CVMultiDec::resume() {
        int size = m_vVDecs.size();
        for (int i = 0; i < size; i++) {
            if (m_vVDecs[i] != NULL) {
                m_vVDecs[i]->resume();
            }
        }
    }

    BOOL32 CVMultiDec::getLayer(CVLayerParam *&pLayer) {
        pLayer = NULL;
        if (!m_bIsFinished && m_sllCurrPlayUS >= m_sllStartPlayUS &&
            (m_sllEndPlayUS < 0 ? TRUE : m_sllCurrPlayUS <= m_sllEndPlayUS)) {
            bool got_frame = false;
            while (!got_frame && !m_bIsFinished) {
                CVLayerParam *tmp = NULL;
                m_vVDecs[m_nIndex]->getLayer(tmp);
                if (tmp == NULL) {
                    if (m_nIndex == m_vVDecs.size() - 1) {
                        LOGE("MultiTrack finished. :) \n");
                        m_bIsFinished = TRUE;
                    } else {
                        LOGE("Child Video Track%u has finished, go to next!!!!!!\n",
                             m_vVDecs[m_nIndex]->getTrackId());
                        m_nIndex++;
                    }
                } else {
                    pLayer = tmp;
                    pLayer->m_uTrackId = m_pTrack->getId();
                    got_frame = true;
                }
            }
        }
        m_sllCurrPlayUS += VIDEO_FRAME_MIN_DURATION;
        return TRUE;
    }
}
