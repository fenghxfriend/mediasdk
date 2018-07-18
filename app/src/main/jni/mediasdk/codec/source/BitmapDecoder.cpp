#include <BitmapDecoder.h>
#include <autolock.h>
#include <enum.h>
#include <track.h>

namespace paomiantv
{

    /**
     * Default Target Duration in microseconds.
     */
    static int64_t s_defaultTargetDuration = 1 * 1000 * 1000;

    BitmapDecoder::BitmapDecoder(ITrack* pTrack)
        :IDecoder(pTrack)
        ,_width(0)
        ,_height(0)
        ,_isPrepared(false)
    {
        initialize();
    }

    BitmapDecoder::~BitmapDecoder()
    {
        reset();
    }

    bool BitmapDecoder::prepare()
    {
        CAutoLock autoLock(_decoderLock);
        if (_isPrepared) {
            return false;
        }
        do {
            if (!_validateTrack(EM_TRACK_VIDEO, EM_SOURCE_BITMAP)) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "BitmapDecoder.prepare:Failed to validate track!");
                break;
            }
            uint8_t *data = nullptr;
            uint32_t size = 0;
            _pTrack->getSourceData(data,size);
            if (!data||size==0) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "BitmapDecoder.prepare:Null data!");
                break;
            }
            _parseTime();
            _width = _pTrack->getWidth();
            _height = _pTrack->getHeight();

            _isPrepared = true;
        } while(false);
        return _isPrepared;
    }

    void BitmapDecoder::release()
    {
        CAutoLock autoLock(_decoderLock);
        if (!_isPrepared) {
            return;
        }
        _isPrepared = false;
        _targetStartTime = 0;
        _targetEndTime = 0;
        _width = 0;
        _height = 0;
    }

    void BitmapDecoder::start()
    {
    }

    void BitmapDecoder::stop()
    {
    }

    void BitmapDecoder::pause()
    {
    }

    void BitmapDecoder::resume()
    {
    }

    void BitmapDecoder::seekTo(int64_t usTime)
    {
    }

    void BitmapDecoder::flush()
    {
    }

    CVLayerParam* BitmapDecoder::getImageLayer(int64_t usPTS)
    {
        if (!_isPrepared) {
            return nullptr;
        }
        if (usPTS < _targetStartTime && !_pTrack->isShowFirstFrame()) {
            return nullptr;
        }
        if (usPTS > _targetEndTime && !_pTrack->isShowLastFrame()) {
            return nullptr;
        }
        return _createBitmapLayer(usPTS);
    }

    CAudio *BitmapDecoder::getAudio(int64_t usPTS)
    {
        return nullptr;
    }

    bool BitmapDecoder::initialize()
    {
        _decoderLock = new CLock();
    }

    void BitmapDecoder::reset()
    {
        if (_decoderLock) {
            delete _decoderLock;
            _decoderLock = nullptr;
        }
    }

    CVLayerParam *BitmapDecoder::_createBitmapLayer(int64_t usPTS)
    {
        CVLayerParam *pLayer = CVLayerParam::create();
        uint8_t *data = nullptr;
        uint32_t size = 0;
        _pTrack->getSourceData(data,size);
        pLayer->resize(size);
        memcpy(pLayer->m_pbyPicture, data, size);
        pLayer->m_uSize = size;
        pLayer->m_sllTimeStampUS = usPTS;
        pLayer->m_uTrackId = _pTrack->getId();
        pLayer->m_uWidth = _pTrack->getWidth();
        pLayer->m_uHeight = _pTrack->getHeight();
        _setLayerParam(pLayer);
        return pLayer;
    }

    void BitmapDecoder::_setLayerParam(CVLayerParam *layerparam)
    {
        s32 asize = ((CTrack *) _pTrack)->getAnimationCount();
        for (int i = 0; i < asize; i++) {
            CAnimation *animation = ((CTrack *) _pTrack)->getAnimation(i);
            s64 offset = 0;
            if (animation != NULL &&
                (offset = (layerparam->m_sllTimeStampUS - _targetStartTime -
                           (animation->m_sllStart * 1000))) >=
                0 && ((animation->m_sllDuration < 0) ? TRUE : (
                    (layerparam->m_sllTimeStampUS - _targetStartTime) <=
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

        s32 esize = ((CTrack *) _pTrack)->getEffectCount();
        for (s32 i = 0; i < esize; i++) {
            CEffect *effect = ((CTrack *) _pTrack)->getEffect(i);
            if (effect != NULL &&
                (layerparam->m_sllTimeStampUS - _targetStartTime - (effect->getStart() * 1000)) >=
                0 && ((effect->getDuration() < 0) ? TRUE : (
                    (layerparam->m_sllTimeStampUS - _targetStartTime) <=
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

    void BitmapDecoder::_parseTime()
    {
        IDecoder::_parseTime();
        if (_pTrack->getPlayDuration() < 0) {
            if (!_pTrack->isIndependent()) {
                _targetEndTime = _targetStartTime + s_defaultTargetDuration;
            } else {
                _targetEndTime = LLONG_MAX;
            }
        }
    }

}