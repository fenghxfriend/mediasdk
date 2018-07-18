//
// Created by ASUS on 2018/5/4.
//

#include "bitmapdec.h"

namespace paomiantv {
    CVBitmapDec::CVBitmapDec(ITrack *const &pTrack) : CDec(pTrack),
                                                      m_uWidth(0),
                                                      m_uHeight(0),
                                                      m_ePixFmt(EM_PIXEL_FORMAT_START),
                                                      m_eDecoderOutputFmt(
                                                              AV_PIX_FMT_NONE),
                                                      m_sllCurrPlayUS(0),
                                                      m_sllStartPlayUS(0),
                                                      m_sllEndCutUS(0) {

    }

    CVBitmapDec::~CVBitmapDec() {
    }

    int CVBitmapDec::prepare() {
        uint8_t *data = nullptr;
        uint32_t size = 0;
        if (m_pTrack != NULL &&
            (m_pTrack->getSourceData(data, size), (data != NULL && size != 0)) &&
            m_pTrack->getType() == EM_TRACK_VIDEO &&
            m_pTrack->isSourceValid() &&
            m_pTrack->getSourceType() == EM_SOURCE_BITMAP) {

            m_pTrack->getSourceData(data, size);
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
            LOGE("the bitmap track is invalid!");
            return -1;
        }
        return 0;
    }

    void CVBitmapDec::release() {

    }

    void CVBitmapDec::start() {
        LOGI("CVBitmapDec::startThread");
    }

    void CVBitmapDec::stop() {
        LOGI("CVBitmapDec::stopThread");
    }

    void CVBitmapDec::pause() {

    }

    void CVBitmapDec::resume() {

    }

    BOOL32 CVBitmapDec::getLayer(CVLayerParam *&pLayer) {
        pLayer = NULL;
        if (m_sllCurrPlayUS >= m_sllStartPlayUS &&
            (m_sllEndPlayUS < 0 ? TRUE : m_sllCurrPlayUS <= m_sllEndPlayUS)) {
            pLayer = CVLayerParam::create();
            uint8_t *data = nullptr;
            uint32_t size = 0;
            m_pTrack->getSourceData(data, size);
            pLayer->resize(size);
            memcpy(pLayer->m_pbyPicture, data,
                   size);
            pLayer->m_uSize = size;
            pLayer->m_sllTimeStampUS = m_sllCurrPlayUS;
            pLayer->m_uTrackId = m_pTrack->getId();
            pLayer->m_sllDurationUS = VIDEO_FRAME_MIN_DURATION;
            pLayer->m_uWidth = m_pTrack->getWidth();
            pLayer->m_uHeight = m_pTrack->getHeight();
            setParams(pLayer);
        }
        m_sllCurrPlayUS += VIDEO_FRAME_MIN_DURATION;
        return TRUE;
    }

    BOOL32 CVBitmapDec::getAudio(CAudio *&pAudio) {
        pAudio = NULL;
        return FALSE;
    }


    void CVBitmapDec::setParams(CVLayerParam *layerparam) {
        s32 asize = ((CTrack *) m_pTrack)->getAnimationCount();
        for (int i = 0; i < asize; i++) {
            CAnimation *animation = ((CTrack *) m_pTrack)->getAnimation(i);
            s64 offset = 0;
            if (animation != NULL &&
                (offset = (layerparam->m_sllTimeStampUS - m_sllStartPlayUS -
                           (animation->m_sllStart * 1000))) >=
                0 && ((animation->m_sllDuration < 0) ? TRUE : (
                    (layerparam->m_sllTimeStampUS - m_sllStartPlayUS) <=
                    ((animation->m_sllStart + animation->m_sllDuration) * 1000)))) {
                offset /= 1000;
                // transform params
                layerparam->m_afTranslate[EM_DIRECT_X] =
                        animation->getVecX() * offset + animation->m_fStartTransX;
                layerparam->m_afTranslate[EM_DIRECT_Y] =
                        animation->getVecY() * offset + animation->m_fStartTransY;
                layerparam->m_afTranslate[EM_DIRECT_Z] =
                        animation->getVecZ() * offset + animation->m_fStartTransZ;

                layerparam->m_afRotate[EM_DIRECT_X] =
                        animation->getVecDegreeX() * offset + animation->m_fStartDegreeX;
                layerparam->m_afRotate[EM_DIRECT_Y] =
                        animation->getVecDegreeY() * offset + animation->m_fStartDegreeY;
                layerparam->m_afRotate[EM_DIRECT_Z] =
                        animation->getVecDegreeZ() * offset + animation->m_fStartDegreeZ;

                layerparam->m_afScale[EM_DIRECT_X] =
                        animation->getVecScaleX() * offset + animation->m_fStartScaleX;
                layerparam->m_afScale[EM_DIRECT_Y] =
                        animation->getVecScaleY() * offset + animation->m_fStartScaleY;
                layerparam->m_afScale[EM_DIRECT_Z] =
                        animation->getVecScaleZ() * offset + animation->m_fStartScaleZ;

                // crop params
                layerparam->m_afUVCropTranslate[EM_DIRECT_X] =
                        animation->getCropVecX() * offset + animation->m_fCropStartTransX;
                layerparam->m_afUVCropTranslate[EM_DIRECT_Y] =
                        animation->getCropVecY() * offset + animation->m_fCropStartTransY;
                layerparam->m_afUVCropTranslate[EM_DIRECT_Z] =
                        animation->getCropVecZ() * offset + animation->m_fCropStartTransZ;

                layerparam->m_afUVCropRotate[EM_DIRECT_X] =
                        animation->getCropVecDegreeX() * offset +
                        animation->m_fCropStartDegreeX;
                layerparam->m_afUVCropRotate[EM_DIRECT_Y] =
                        animation->getCropVecDegreeY() * offset +
                        animation->m_fCropStartDegreeY;
                layerparam->m_afUVCropRotate[EM_DIRECT_Z] =
                        animation->getCropVecDegreeZ() * offset +
                        animation->m_fCropStartDegreeZ;

                layerparam->m_afUVCropScale[EM_DIRECT_X] =
                        animation->getCropVecScaleX() * offset +
                        animation->m_fCropStartScaleX;
                layerparam->m_afUVCropScale[EM_DIRECT_Y] =
                        animation->getCropVecScaleY() * offset +
                        animation->m_fCropStartScaleY;
                layerparam->m_afUVCropScale[EM_DIRECT_Z] =
                        animation->getCropVecScaleZ() * offset +
                        animation->m_fCropStartScaleZ;

                // color alpha
                layerparam->m_fAlpha =
                        animation->getVecAlpha() * offset + animation->m_fStartAlpha;

            }
        }

        s32 esize = ((CTrack *) m_pTrack)->getEffectCount();
        for (s32 i = 0; i < esize; i++) {
            CEffect *effect = ((CTrack *) m_pTrack)->getEffect(i);
            if (effect != NULL &&
                (layerparam->m_sllTimeStampUS - m_sllStartPlayUS - (effect->getStart() * 1000)) >=
                0 && ((effect->getDuration() < 0) ? TRUE : (
                    (layerparam->m_sllTimeStampUS - m_sllStartPlayUS) <=
                    ((effect->getStart() + effect->getDuration()) * 1000)))) {
                CVFilterParam *filterParam = CVFilterParam::create();
                filterParam->reset();
                effect->getFilter(filterParam->m_pFilterSource, filterParam->m_eType);
                if (filterParam->m_pFilterSource != NULL) {
                    layerparam->m_vFilterParams.push_back(filterParam);
                } else {
                    CVFilterParam::release(filterParam);
                }
            }
        }
    }

    BOOL32 CVBitmapDec::getRemainderBuffer(u8 *&out, u32 &size) {
        out = NULL;
        size = 0;
        return FALSE;
    }
}


