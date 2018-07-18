//
// Created by ASUS on 2018/4/12.
//

#include <autolog.h>
#include <cstdlib>
#include <constant.h>
#include <track.h>
#include <transition.h>
#include <webp/decode.h>
#include <webp/demux.h>
#include "../../../webp/imageio/imageio_util.h"

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
}
#endif

namespace paomiantv {

    // ITrack
    u32 ITrack::m_sCount = 1;

    void ITrack::setType(EMTrack type) {
        CAutoLock autoLock(m_pLock);
        this->m_eType = type;
    }

    void ITrack::setPlayStart(s64 sllPlayStart) {
        CAutoLock autoLock(m_pLock);
        m_sllPlayStart = sllPlayStart;
    }

    void ITrack::setPlayDuration(s64 sllPlayDuration) {
        CAutoLock autoLock(m_pLock);
        m_sllPlayDuration = sllPlayDuration;
    }


    void ITrack::setZIndex(s16 wZIndex) {
        CAutoLock autoLock(m_pLock);
        m_wZIndex = wZIndex;
    }

    void ITrack::setWeight(s16 wWeight) {
        CAutoLock autoLock(m_pLock);
        m_wWeight = wWeight;
    }

    void ITrack::setShowFirstFrame(BOOL32 bShow) {
        CAutoLock autoLock(m_pLock);
        m_bIsShowFirstFrame = bShow;
    }

    void ITrack::setShowLastFrame(BOOL32 bShow) {
        CAutoLock autoLock(m_pLock);
        m_bIsShowLastFrame = bShow;
    }

    void ITrack::setIndependent(BOOL32 bIndependent) {
        CAutoLock autoLock(m_pLock);
        m_bIsIndependent = bIndependent;
    }

    void ITrack::setLoop(BOOL32 bIsLoop) {
        CAutoLock autoLock(m_pLock);
        m_bIsLoop = bIsLoop;
    }

    BOOL32 ITrack::isSourceValid() const {
        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        if (m_ptTrackSource != NULL) {
            re = m_ptTrackSource->m_bIsValid;
        }
        return re;
    }

    void ITrack::getSourceData(u8 *&pbyData, u32 &uSize) {
        pbyData = NULL;
        uSize = 0;
        CAutoLock autoLock(m_pLock);
        if (m_ptTrackSource != NULL) {
            pbyData = m_ptTrackSource->m_pbyData;
            uSize = m_ptTrackSource->m_uSize;
        }
    }

    EMSource ITrack::getSourceType() const {
        EMSource re = EM_SOURCE_START;
        CAutoLock autoLock(m_pLock);
        if (m_ptTrackSource != NULL) {
            re = m_ptTrackSource->m_eSourceType;
        }
        return re;
    }

    u32 ITrack::getWidth() {
        u32 re = 0;
        CAutoLock autoLock(m_pLock);
        if (m_ptTrackSource != NULL) {
            re = m_ptTrackSource->m_uWidth;
        }
        return re;
    }

    u32 ITrack::getHeight() {
        u32 re = 0;
        CAutoLock autoLock(m_pLock);
        if (m_ptTrackSource != NULL) {
            re = m_ptTrackSource->m_uHeight;
        }
        return re;
    }


    bool ITrack::compareByZindexCB(ITrack *t1, ITrack *t2) {
        return (t1->getZIndex() > t2->getZIndex());
    }

    bool ITrack::compareByIdCB(u64 id, ITrack *pTrack) {
        return (id == pTrack->getId());
    }


    //CTrack
    CTrack::CTrack() {
        USE_LOG;
        m_sllCutStart = 0;
        m_sllCutDuration = -1;
        m_sllOriginDurationUS = 0;
        m_sllDataDurationUS = 0;
        m_bIsLoop = FALSE;
        m_fPlayRate = 1.0f;
        m_fVolume = 1.0f;
        m_pvEffectLock = new CLock;
        m_pvAnimationLock = new CLock;
        m_vEffects.clear();
        m_vAnimations.clear();
    }

    CTrack::~CTrack() {
        USE_LOG;
        if (m_pvAnimationLock != NULL) {
            delete m_pvAnimationLock;
        }
        if (m_pvEffectLock != NULL) {
            delete m_pvEffectLock;
        }
    }

    BOOL32
    CTrack::setDataSource(const EMSource eType, const u8 *data, const u32 size, const u32 width,
                          const u32 height) {
        CAutoLock autoLock(m_pLock);
        if (m_ptTrackSource->m_bIsValid) {
            u8 *p = NULL;
            p = (u8 *) realloc(m_ptTrackSource->m_pbyData, size + 1);
            if (p == NULL) {
                return FALSE;
            }
            m_ptTrackSource->m_pbyData = p;
        }
        m_ptTrackSource->m_eSourceType = eType;
        m_ptTrackSource->m_pbyData = (u8 *) malloc(size + 1);
        memcpy(m_ptTrackSource->m_pbyData, data, size);
        m_ptTrackSource->m_pbyData[size] = '\0';
        m_ptTrackSource->m_uSize = size;
        m_ptTrackSource->m_bIsValid = TRUE;
        m_ptTrackSource->m_uWidth = width;
        m_ptTrackSource->m_uHeight = height;
        if (parse() != 0) {
            return FALSE;
        }
        return TRUE;

    }

    void CTrack::setCutStart(s64 sllStart) {
        CAutoLock autoLock(m_pLock);
        m_sllCutStart = sllStart;
        updateDataDuration();
    }

    void CTrack::setCutDuration(s64 sllDuration) {
        CAutoLock autoLock(m_pLock);
        m_sllCutDuration = sllDuration;
        updateDataDuration();
    }

    void CTrack::setPlayRate(float fRate) {
        CAutoLock autoLock(m_pLock);
        m_fPlayRate = fRate;
    }

    void CTrack::setVolume(float fVolume) {
        CAutoLock autoLock(m_pLock);
        if (fVolume < 0.f || fVolume > 4.0f) {
            return;
        }
        m_fVolume = fVolume;
    }

    CEffect *CTrack::getEffect(s32 nIndex) const {
        CEffect *pEffect = NULL;
        BEGIN_AUTOLOCK(m_pvEffectLock);
            do {
                if (nIndex >= m_vEffects.size()) {
                    break;
                }
                pEffect = m_vEffects.at(nIndex);
            } while (0);
        END_AUTOLOCK;
        return pEffect;
    }

    BOOL32 CTrack::addEffect(CEffect *pEffect) {
        BOOL32 re = FALSE;
        BEGIN_AUTOLOCK(m_pvEffectLock);
            do {
                if (pEffect == NULL) {
                    break;
                }
                m_vEffects.push_back(pEffect);
                re = TRUE;
            } while (0);
        END_AUTOLOCK;
        return re;
    }

    CEffect *CTrack::removeEffectByIndex(s32 nIndex) {
        CEffect *pFilter = NULL;
        BEGIN_AUTOLOCK(m_pvEffectLock);
            do {
                if (nIndex >= m_vEffects.size()) {
                    break;
                }
                pFilter = m_vEffects.at(nIndex);
                m_vEffects.erase(m_vEffects.begin() + nIndex);
            } while (0);
        END_AUTOLOCK;
        return pFilter;
    }

    CEffect *CTrack::removeEffectById(s32 id) {
        CEffect *pFilter = NULL;
        BEGIN_AUTOLOCK(m_pvEffectLock);
            do {
                bool find = false;
                u32 index = 0;
                for (u32 i = 0; i < m_vEffects.size(); ++i) {
                    if (m_vEffects[i] != NULL && m_vEffects[i]->getId() == id) {
                        find = true;
                        index = i;
                        break;
                    }
                }
                if (find) {
                    pFilter = m_vEffects.at(index);
                    m_vEffects.erase(m_vEffects.begin() + index);
                }
            } while (0);
        END_AUTOLOCK;
        return pFilter;
    }


    BOOL32 CTrack::addAnimation(CAnimation *pAnimation) {
        BOOL32 re = FALSE;
        BEGIN_AUTOLOCK(m_pvAnimationLock);
            do {
                if (pAnimation == NULL) {
                    break;
                }
                m_vAnimations.push_back(pAnimation);
                re = TRUE;
            } while (0);
        END_AUTOLOCK;
        return re;
    }

    CAnimation *CTrack::getAnimation(s32 nIndex) const {
        CAnimation *pAnimation = NULL;
        BEGIN_AUTOLOCK(m_pvAnimationLock);
            do {
                if (nIndex >= m_vAnimations.size()) {
                    break;
                }
                pAnimation = m_vAnimations.at(nIndex);
            } while (0);
        END_AUTOLOCK;
        return pAnimation;
    }

    CAnimation *CTrack::removeAnimationByIndex(s32 nIndex) {
        CAnimation *pAnimation = NULL;
        BEGIN_AUTOLOCK(m_pvAnimationLock);
            do {
                if (nIndex >= m_vAnimations.size()) {
                    break;
                }
                pAnimation = m_vAnimations.at(nIndex);
                m_vAnimations.erase(m_vAnimations.begin() + nIndex);
            } while (0);
        END_AUTOLOCK;
        return pAnimation;
    }

    CAnimation *CTrack::removeAnimationById(s32 id) {
        CAnimation *pAnimation = NULL;
        BEGIN_AUTOLOCK(m_pvAnimationLock);
            do {
                bool find = false;
                u32 index = 0;
                for (u32 i = 0; i < m_vAnimations.size(); ++i) {
                    if (m_vAnimations[i] != NULL && m_vAnimations[i]->getId() == id) {
                        find = true;
                        index = i;
                        break;
                    }
                }
                if (find) {
                    pAnimation = m_vAnimations.at(index);
                    m_vAnimations.erase(m_vAnimations.begin() + index);
                }
            } while (0);
        END_AUTOLOCK;
        return pAnimation;
    }

    s64 CTrack::getDataDuration() {
        return m_sllDataDurationUS;
    }

    int CTrack::parse() {
        int error = 0;
        switch (m_ptTrackSource->m_eSourceType) {
            case EM_SOURCE_FILE: {
                BOOL32 re = FALSE;
                AVFormatContext *ptAVFormatCxt = NULL;
                do { /** Open the input file to read from it. */
                    if ((error = avformat_open_input(&ptAVFormatCxt,
                                                     (s8 *) m_ptTrackSource->m_pbyData,
                                                     NULL,
                                                     NULL)) < 0) {
                        LOGE("Could not open input file '%s' (error '%d')\n",
                             (s8 *) m_ptTrackSource->m_pbyData, error);
                        ptAVFormatCxt = NULL;
                        break;
                    }
                    /** Get information on the input file (number of streams etc.). */
                    if ((error = avformat_find_stream_info(ptAVFormatCxt, NULL)) < 0) {
                        LOGE("Could not open find stream info (error '%d')\n", error);
                        break;
                    }
                    m_sllOriginDurationUS = ptAVFormatCxt->duration;

                    if ((error = updateDataDuration()) < 0) {
                        break;
                    }

                    // check if the source file has the audio/video stream
                    enum AVMediaType type = AVMEDIA_TYPE_UNKNOWN;
                    switch (m_eType) {
                        case EM_TRACK_AUDIO:
                            type = AVMEDIA_TYPE_AUDIO;
                            break;
                        case EM_TRACK_VIDEO:
                            type = AVMEDIA_TYPE_VIDEO;
                            break;
                        default:
                            break;
                    }
                    if ((type == AVMEDIA_TYPE_AUDIO || type == AVMEDIA_TYPE_VIDEO) &&
                        ((error = av_find_best_stream(ptAVFormatCxt, type, -1, -1,
                                                      NULL, 0)) < 0)) {
                        LOGE("Could not find %s stream in input file '%s'\n",
                             av_get_media_type_string(type), (s8 *) m_ptTrackSource->m_pbyData);
                        break;
                    }
                    if (type == AVMEDIA_TYPE_VIDEO) {
                        m_ptTrackSource->m_uWidth = ptAVFormatCxt->streams[error]->codecpar->width;
                        m_ptTrackSource->m_uHeight = ptAVFormatCxt->streams[error]->codecpar->height;
                    }
                    re = TRUE;
                } while (0);
                if (ptAVFormatCxt != NULL) {
                    avformat_close_input(&ptAVFormatCxt);
                }
                if (!re) {
                    return error;
                } else {
                    error = 0;
                }
            }
                break;
            case EM_SOURCE_WEBP: {
                BOOL32 re = FALSE;
                WebPData tData;
                memset(&tData, 0, sizeof(tData));
                WebPDemuxer *ptDmux = NULL;
                do {
                    if (!ImgIoUtilReadFile((s8 *) m_ptTrackSource->m_pbyData,
                                           &tData.bytes, &tData.size)) {
                        LOGE("Get webp data from source file failed.\n");
                        error = AVERROR_INVALIDDATA;
                        break;
                    }

                    if (!WebPGetInfo(tData.bytes, tData.size, NULL, NULL)) {
                        LOGE("Input file doesn't appear to be WebP format.\n");
                        error = AVERROR_INVALIDDATA;
                        break;
                    }

                    ptDmux = WebPDemux(&tData);
                    if (ptDmux == NULL) {
                        LOGE("Could not create demuxing object!\n");
                        error = AVERROR_INVALIDDATA;
                        break;
                    }
                    u32 m_uFrameCount = WebPDemuxGetI(ptDmux, WEBP_FF_FRAME_COUNT);
                    WebPIterator curr;
                    for (size_t i = 1; i <= m_uFrameCount; i++) {

                        if (!WebPDemuxGetFrame(ptDmux, i, &curr)) {// Get ith frame.
                            LOGE("Could not retrieve frame# %d", i);
                            error = AVERROR_INVALIDDATA;
                            break;
                        }
                        m_sllOriginDurationUS += (curr.duration * 1000);
                    }
                    if (error != 0) {
                        break;
                    }
                    if ((error = updateDataDuration()) < 0) {
                        break;
                    }
                    m_ptTrackSource->m_uWidth = WebPDemuxGetI(ptDmux, WEBP_FF_CANVAS_WIDTH);
                    m_ptTrackSource->m_uHeight = WebPDemuxGetI(ptDmux, WEBP_FF_CANVAS_HEIGHT);
                    re = TRUE;
                } while (0);
                if (ptDmux != NULL) {
                    WebPDemuxDelete(ptDmux);
                }
                if (tData.bytes != NULL) {
                    delete[] tData.bytes;
                }
                if (!re) {
                    return error;
                }
            }
                break;
            case EM_SOURCE_BITMAP: {
                m_sllOriginDurationUS = INTEGER32_MAX_VALUE;
                updateDataDuration();
            }
                break;
            case EM_SOURCE_SILENCE: {
                m_sllOriginDurationUS = INTEGER32_MAX_VALUE;
                updateDataDuration();
            }
                break;
            default: {
                LOGE("invalid source!");
                error = AVERROR_INVALIDDATA;
            }
                break;
        }

        return error;
    }

    int CTrack::updateDataDuration() {
        if (m_sllCutStart < 0 || m_sllCutStart * 1000 >= m_sllOriginDurationUS) {
            LOGE("track params is invalid\n");
            m_sllDataDurationUS = 0;
            return AVERROR_INVALIDDATA;
        }
        s64 endCutUS = m_sllOriginDurationUS;
        if (m_sllCutDuration >= 0) {
            endCutUS = MIN ((m_sllCutStart + m_sllCutDuration) * 1000, endCutUS);
        }

        m_sllDataDurationUS = endCutUS - (m_sllCutStart * 1000);

        return 0;
    }

} // namespace paomiantv