#include <VideoMultiDecoder.h>
#include <enum.h>
#include <track.h>
#include <multitrack.h>
#include <VideoDecoder.h>
#include <BitmapDecoder.h>
#include <WebpDecoder.h>

namespace paomiantv
{

    VideoMultiDecoder::VideoMultiDecoder(ITrack* pTrack)
        :IDecoder(pTrack)
        ,_decoders()
        ,_currentIndex(0)
        ,_decoderLock(nullptr)
        ,_isFinished(false)
    {
        _decoderLock = new CLock;
    }

    VideoMultiDecoder::~VideoMultiDecoder()
    {
        release();
        if (_decoderLock != nullptr) {
            delete _decoderLock;
            _decoderLock = nullptr;
        }
    }

#if 1
    bool VideoMultiDecoder::prepare()
    {
        CAutoLock autoLock(_decoderLock);
        std::string errorInfo = "";
        do {
            if (!_validateTrack(EM_TRACK_VIDEO, EM_SOURCE_MULTI)) {
                errorInfo = "Invalid Track!";
                break;
            }
            _parseTime();

            CMultiTrack* multiTrack = dynamic_cast<CMultiTrack*>(_pTrack);
            if (!multiTrack) {
                errorInfo = "Invalid track instance!";
                break;
            }
            int trackCount = multiTrack->getTrackCount();
            std::vector<IDecoder*> decoderList;
            int64_t subStartTime = _targetStartTime;
            for (int i=0; i<trackCount; ++i) {
                ITrack *pTrack = nullptr;
                multiTrack->getTrack(pTrack, i);
                IDecoder *pDecoder = _createSubDecoder(subStartTime, pTrack);
                if (pDecoder) {
                    subStartTime = pDecoder->getTargetEndTime();
                    decoderList.push_back(pDecoder);
                }
            }
            if (decoderList.empty()) {
                errorInfo = "Got none sub decoders!";
                break;
            }
            std::swap(_decoders, decoderList);
            _decoderState = EDecoderStopped;
            _isFinished = false;
        } while(false);

        if (!errorInfo.empty()) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "VideoMultiDecoder.prepare:%s", errorInfo.c_str());
            return false;
        }
        return true;
    }
#else
    bool VideoMultiDecoder::prepare()
    {
        std::string errorInfo = "";
        do {
            if (!_pTrack) {
                errorInfo = "Track is null!";
                break;
            }
            if (!_pTrack->getSource()) {
                errorInfo = "Source of track is null!";
                break;
            }
            if (EM_TRACK_VIDEO != _pTrack->getType()) {
                errorInfo = "Track-type is invalid!";
                break;
            }
            if (EM_SOURCE_MULTI != _pTrack->getSource()->m_eSourceType) {
                errorInfo = "Source-type is invalid!";
                break;
            }
            CMultiTrack* multiTrack = (CMultiTrack*)_pTrack;
            int trackCount = multiTrack->getTrackCount();
            std::vector<IDecoder*> decoderList;
            for (int i=0; i<trackCount; ++i) {
                ITrack *pTrack = nullptr;
                if (multiTrack->getTrack(pTrack, i) &&
                    pTrack != nullptr &&
                    pTrack->getType() == EM_TRACK_VIDEO &&
                    pTrack->getSource() != nullptr &&
                    pTrack->getSource()->m_pbyData != nullptr) {
                    IDecoder *curDecoder = new VideoDecoder(pTrack);
                    if(curDecoder->prepare()) {
                        decoderList.push_back(curDecoder);
                    } else {
                        errorInfo = "Failed prepare sub track!";
                        break;
                    }
                } else {
                    errorInfo = "Sub track is invalid!";
                    break;
                }
            }
            if (!errorInfo.empty()) {
                for (auto pDecoder : decoderList) {
                    if (pDecoder) {
                        delete pDecoder;
                    }
                }
                decoderList.clear();
                break;
            }

            std::swap(_decoders, decoderList);
            _targetStartTime = _pTrack->getPlayStart() * 1000;
            int64_t dataDuration = _pTrack->getDataDuration();
            int64_t playDuration = _pTrack->getPlayDuration() < 0 ? dataDuration : _pTrack->getPlayDuration() * 1000;
            _targetEndTime = _targetStartTime + std::min(dataDuration, playDuration);
            _isFinished = false;
        } while(false);

        if (!errorInfo.empty()) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "VideoMultiDecoder.prepare:%s", errorInfo.c_str());
            return false;
        }
        return true;
    }
#endif

    void VideoMultiDecoder::release()
    {
        CAutoLock autoLock(_decoderLock);
        std::vector<IDecoder*> deleteList;
        std::swap(deleteList, _decoders);
        for (size_t i=0; i<deleteList.size(); ++i) {
            if (deleteList[i]) {
                deleteList[i]->release();
                delete deleteList[i];
            }
        }
        deleteList.clear();
        _decoderState = EDecoderInvalid;
    }

    void VideoMultiDecoder::start()
    {
        CAutoLock autoLock(_decoderLock);
        for (size_t i=0; i<_decoders.size(); ++i) {
            if (_decoders[i]) {
                _decoders[i]->start();
            }
        }
        switch (_decoderState) {
            case EDecoderStopped:
                _decoderState = EDecoderPlaying;
                break;
            default:
                break;
        }
    }

    void VideoMultiDecoder::stop()
    {
        CAutoLock autoLock(_decoderLock);
        for (size_t i=0; i<_decoders.size(); ++i) {
            if (_decoders[i]) {
                _decoders[i]->stop();
            }
        }
        switch (_decoderState) {
            case EDecoderPlaying:
            case EDecoderPaused: {
                _decoderState = EDecoderStopped;
            }
                break;
            default:
                break;
        }
    }

    void VideoMultiDecoder::pause()
    {
        CAutoLock autoLock(_decoderLock);
        for (size_t i=0; i<_decoders.size(); ++i) {
            if (_decoders[i]) {
                _decoders[i]->pause();
            }
        }
        switch (_decoderState) {
            case EDecoderPlaying:
                _decoderState = EDecoderPaused;
                break;
            default:
                break;
        }
    }

    void VideoMultiDecoder::resume()
    {
        CAutoLock autoLock(_decoderLock);
        for (size_t i=0; i<_decoders.size(); ++i) {
            if (_decoders[i]) {
                _decoders[i]->resume();
            }
        }
        switch (_decoderState) {
            case EDecoderPaused:
                _decoderState = EDecoderPlaying;
                break;
            default:
                break;
        }
    }

    void VideoMultiDecoder::seekTo(int64_t usTime)
    {
        CAutoLock autoLock(_decoderLock);
        if (EDecoderInvalid == _decoderState) {
            return;
        }

        if (usTime < _targetStartTime || usTime >= _targetEndTime) {
            _currentIndex = 0;
        } else {
            for (size_t i = 0; i < _decoders.size(); ++i) {
                if (usTime >= _decoders[i]->getTargetStartTime() &&
                    usTime < _decoders[i]->getTargetEndTime()) {
                    _currentIndex = i;
                    break;
                }
            }
        }

        for (IDecoder *pDecoder : _decoders) {
            if (pDecoder) {
                pDecoder->seekTo(usTime);
            }
        }
    }

    bool VideoMultiDecoder::isSeeking() {
        for (auto pDecoder : _decoders) {
            if (pDecoder->isSeeking()) {
                return true;
            }
        }
        _isFinished = false;
        return false;
    }

    void VideoMultiDecoder::flush()
    {
        //Reserved.
    }

    CVLayerParam *VideoMultiDecoder::getImageLayer(int64_t usPTS)
    {
        if (EDecoderStopped >= _decoderState) {
            return nullptr;
        }
        CVLayerParam *pLayer = nullptr;
        do {
            if (_isFinished) {
                //TODO:log warning
                break;
            }
            if (_currentIndex >= _decoders.size()) {
                //TODO:Log warning
                break;
            }
            if (usPTS < _targetStartTime && !_pTrack->isShowFirstFrame()) {
                break;
            }
            if (usPTS > _targetEndTime && !_pTrack->isShowLastFrame()) {
                break;
            }
            while (!_isFinished) {
                if (EDecoderStopped >= _decoderState) {
                    break;
                }
                pLayer = _decoders[_currentIndex]->getImageLayer(usPTS);
                if (pLayer) {
                    pLayer->m_uTrackId = _pTrack->getId();
                    break;
                } else {
                    if (_currentIndex >= (_decoders.size() - 1)) {
                        _isFinished = true;
                    } else {
                        //TODO:log debug
                        ++_currentIndex;
                    }
                }
            }
        } while(false);
        return pLayer;
    }

    CAudio *VideoMultiDecoder::getAudio(int64_t usPTS)
    {
        //TODO:log warning
        return nullptr;
    }

    IDecoder* VideoMultiDecoder::_createSubDecoder(int64_t targetStartTime, ITrack *srcTrack)
    {
        if (!srcTrack || !srcTrack->isSourceValid()) {
            //TODO:log error
            return nullptr;
        }

        IDecoder *pDecoder = nullptr;
        switch (srcTrack->getSourceType()) {
            case EM_SOURCE_MULTI:
                pDecoder = new VideoMultiDecoder(srcTrack);
                break;
            case EM_SOURCE_FILE:
                pDecoder = new VideoDecoder(srcTrack);
                break;
            case EM_SOURCE_BITMAP:
                pDecoder = new BitmapDecoder(srcTrack);
                break;
            case EM_SOURCE_WEBP:
                pDecoder = new WebpDecoder(srcTrack);
                break;
            default:
                break;
        }
        if (!pDecoder || !pDecoder->prepare()) {
            //TODO:log error
            delete pDecoder;
            pDecoder = nullptr;
        } else {
            pDecoder->setShowFirstFrame(true);
            int64_t targetDuration = pDecoder->getTargetEndTime() - pDecoder->getTargetStartTime();
            pDecoder->setTargetTime(targetStartTime, targetDuration);
        }
        return pDecoder;
    }

    void VideoMultiDecoder::_parseTime()
    {
        _targetStartTime = _pTrack->getPlayStart() * 1000;
        int64_t dataDuration = _pTrack->getDataDuration();
        int64_t playDuration = _pTrack->getPlayDuration() < 0 ? dataDuration : _pTrack->getPlayDuration() * 1000;
        _targetEndTime = _targetStartTime + std::min(dataDuration, playDuration);
    }

}