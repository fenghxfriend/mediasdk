#include "IDecoder.h"
#include <track.h>

namespace paomiantv
{

    static const int s_defaultCacheCount = 5;

    /**
     * Default minmum frame interval, in microseconds.
     */
    static const int s_defaultMinInterval = 33 * 1000;

    IDecoder::IDecoder(ITrack *mediaTrack)
        :_pTrack(mediaTrack)
        ,_cacheCount(s_defaultCacheCount)
        ,_minInterval(s_defaultMinInterval)
        ,_currentTime(0)
        ,_decoderState(EDecoderInvalid)
        ,_showFirstFrame(mediaTrack ? mediaTrack->isShowFirstFrame() : false)
        ,_showLastFrame(mediaTrack ? mediaTrack->isShowLastFrame() : false)
        ,_isLoop(false)
    {

    }

    IDecoder::~IDecoder() {

    }

    uint32_t IDecoder::getTrackID() {
        return _pTrack ? _pTrack->getId() : 0;
    }

    int16_t IDecoder::getWeight() {
        return _pTrack ? _pTrack->getWeight() : (int16_t)1;
    }

    EMSource IDecoder::getSourceType() {
        return _pTrack ? _pTrack->getSourceType() : EM_SOURCE_START;
    }

    void IDecoder::setSourceTime(int64_t startTime, int64_t durTime)
    {
        _sourceStartTime = startTime;
        _sourceEndTime = _sourceStartTime + durTime;
    }

    void IDecoder::setTargetTime(int64_t startTime, int64_t durTime)
    {
        _targetStartTime = startTime;
        _targetEndTime = _targetStartTime + durTime;
    }

    bool IDecoder::_validateTrack(EMTrack trackType, EMSource sourceType)
    {
        if (!_pTrack) {
            //TODO:log error
            return false;
        }
        if (!_pTrack->isSourceValid()) {
            //TODO:log error
            return false;
        }
        if (trackType != _pTrack->getType()) {
            //TODO:log error
            return false;
        }
        if (sourceType != _pTrack->getSourceType()) {
            //TODO:log error
            return false;
        }
        return true;
    }

    void IDecoder::_parseTime()
    {
        if (!_pTrack) {
            return;
        }

        int64_t dataDuration = ((CTrack*)_pTrack)->getDataDuration();
        _sourceStartTime = ((CTrack*)_pTrack)->getCutStart() * 1000;
        _sourceEndTime = _sourceStartTime + dataDuration;

        _targetStartTime = ((CTrack*)_pTrack)->getPlayStart() * 1000;
        int64_t targetDuration = _pTrack->getPlayDuration() < 0 ? dataDuration : _pTrack->getPlayDuration() * 1000;
        _targetEndTime = _targetStartTime + targetDuration;
    }

}