#include <AudioMultiDecoder.h>
#include <AudioDecoder.h>
#include <multitrack.h>
#include <sound.h>
#include <SilenceDecoder.h>


namespace paomiantv {

    AudioMultiDecoder::AudioMultiDecoder(ITrack *pTrack)
            : IDecoder(pTrack),
              _audioDecoders(),
              _decoderLock(nullptr),
              _currentIndex(0),
              _audioBufferPool(nullptr),
              _audioBufferCapacity(0),
              _audioBufferSize(0),
              _trunkSize(0),
              _isFinished(false) {
        _decoderLock = new CLock;
    }

    AudioMultiDecoder::~AudioMultiDecoder() {
        release();
        if (_decoderLock != nullptr) {
            delete _decoderLock;
            _decoderLock = nullptr;
        }
    }

#if 1

    bool AudioMultiDecoder::prepare() {
        CAutoLock autoLock(_decoderLock);
        std::string errorInfo = "";
        do {
            if (!_validateTrack(EM_TRACK_AUDIO, EM_SOURCE_MULTI)) {
                errorInfo = "Failed to validate track";
                break;
            }
            _parseTime();

            CMultiTrack *multiTrack = dynamic_cast<CMultiTrack *>(_pTrack);
            if (!multiTrack) {
                errorInfo = "Invalid track instance!";
                break;
            }

            int trackCount = multiTrack->getTrackCount();
            std::vector<IDecoder *> decoderList;
            int64_t subStartTime = _targetStartTime;
            for (int i = 0; i < trackCount; ++i) {
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
            std::swap(_audioDecoders, decoderList);

            _trunkSize = AUDIO_SAMPLE_COUNT_PER_FRAME * (0x0001 << AudioDecoder::s_sampleFormat);
            _trunkSize *= av_get_channel_layout_nb_channels(AudioDecoder::s_channelLayout);
            _audioBufferPool = (uint8_t *) malloc(_trunkSize);
            _audioBufferSize = 0;
            _decoderState = EDecoderStopped;
            _isFinished = false;
        } while (false);

        if (!errorInfo.empty()) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "AudioMultiDecoder.prepare:%s",
                                errorInfo.c_str());
            return false;
        }
        return true;
    }

#else
    bool AudioMultiDecoder::prepare()
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
            if (EM_TRACK_AUDIO != _pTrack->getType()) {
                errorInfo = "Track-type is invalid!";
                break;
            }
            if (EM_SOURCE_MULTI != _pTrack->getSource()->m_eSourceType) {
                errorInfo = "Source-type is invalid!";
                break;
            }
            CMultiTrack* multiTrack = (CMultiTrack*)_pTrack;
            int trackCount = multiTrack->getTrackCount();
            std::vector<AudioDecoder*> decoderList;
            for (int i=0; i<trackCount; ++i) {
                ITrack *pTrack = nullptr;
                if (multiTrack->getTrack(pTrack, i) &&
                    pTrack != nullptr &&
                    pTrack->getType() == EM_TRACK_VIDEO &&
                    pTrack->getSource() != nullptr &&
                    pTrack->getSource()->m_pbyData != nullptr) {
                    AudioDecoder *curDecoder = new AudioDecoder(pTrack);
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

            std::swap(_audioDecoders, decoderList);
            _targetStartTime = _pTrack->getPlayStart() * 1000;
            int64_t dataDuraiton = _pTrack->getDataDuration() * 1000;
            _targetEndTime = _pTrack->getPlayDuration() < 0 ? dataDuraiton : (_targetStartTime + _pTrack->getPlayDuration() * 1000);

            _trunkSize = AUDIO_SAMPLE_COUNT_PER_FRAME * (0x0001 << AudioDecoder::s_sampleFormat);
            _trunkSize *= av_get_channel_layout_nb_channels(AudioDecoder::s_channelLayout);
            _audioBufferPool = (uint8_t*)malloc(_trunkSize);
            _audioBufferSize = 0;

            _isFinished = false;
        } while(false);

        if (!errorInfo.empty()) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "AudioMultiDecoder.prepare:%s", errorInfo.c_str());
            return false;
        }
        return true;
    }
#endif

    void AudioMultiDecoder::release() {
        CAutoLock autoLock(_decoderLock);
        std::vector<IDecoder *> deleteList;
        std::swap(deleteList, _audioDecoders);
        for (size_t i = 0; i < deleteList.size(); ++i) {
            if (deleteList[i]) {
                deleteList[i]->release();
                delete deleteList[i];
            }
        }
        deleteList.clear();
        _decoderState = EDecoderInvalid;
    }

    void AudioMultiDecoder::start() {
        CAutoLock autoLock(_decoderLock);
        for (size_t i = 0; i < _audioDecoders.size(); ++i) {
            if (_audioDecoders[i]) {
                _audioDecoders[i]->start();
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

    void AudioMultiDecoder::stop() {
        CAutoLock autoLock(_decoderLock);
        for (size_t i = 0; i < _audioDecoders.size(); ++i) {
            if (_audioDecoders[i]) {
                _audioDecoders[i]->stop();
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

    void AudioMultiDecoder::pause() {
        CAutoLock autoLock(_decoderLock);
        for (size_t i = 0; i < _audioDecoders.size(); ++i) {
            if (_audioDecoders[i]) {
                _audioDecoders[i]->pause();
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

    void AudioMultiDecoder::resume() {
        CAutoLock autoLock(_decoderLock);
        for (size_t i = 0; i < _audioDecoders.size(); ++i) {
            if (_audioDecoders[i]) {
                _audioDecoders[i]->resume();
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

    static int64_t _getTargetStartTime(ITrack *srcTrack) {
        int64_t dataDuration = srcTrack->getDataDuration();

    }

#if 1

    void AudioMultiDecoder::seekTo(int64_t usTime) {
        CAutoLock autoLock(_decoderLock);
        if (EDecoderInvalid == _decoderState) {
            return;
        }
        if (usTime < _targetStartTime || usTime >= _targetEndTime) {
            _currentIndex = 0;
        } else {
            for (size_t i = 0; i < _audioDecoders.size(); ++i) {
                if (usTime >= _audioDecoders[i]->getTargetStartTime() &&
                    usTime < _audioDecoders[i]->getTargetEndTime()) {
                    _currentIndex = i;
                    break;
                }
            }
        }
        for (IDecoder *pDecoder : _audioDecoders) {
            if (pDecoder) {
                pDecoder->seekTo(usTime);
            }
        }
    }

#else
    void AudioMultiDecoder::seekTo(int64_t usTime)
    {
        CMultiTrack *multiTrack = (CMultiTrack*)_pTrack;
        if (!_pTrack || multiTrack->getTrackCount() == 0) {
            //Log error
            return;
        }
        int64_t startTime = multiTrack->getPlayStart() * 1000;
        int64_t endTime = multiTrack->getPlayDuration() > 0 ? multiTrack->getPlayDuration() * 1000 : -1;
        for (uint32_t i=0; i<multiTrack->getTrackCount(); ++i) {
            ITrack *curTrack = nullptr;
            multiTrack->getTrack(curTrack, i);
            if (!curTrack) {
                continue;
            }
            //TODO:Need to finish it.
        }
    }
#endif

    bool AudioMultiDecoder::isSeeking() {
        for (auto pDecoder : _audioDecoders) {
            if (pDecoder->isSeeking()) {
                return true;
            }
        }
        _audioBufferSize = 0;
        _isFinished = false;
        return false;
    }

    void AudioMultiDecoder::flush() {
        //Reserved.
    }

    CVLayerParam *AudioMultiDecoder::getImageLayer(int64_t usPTS) {
        //TODO:log warning
        return nullptr;
    }

    CAudio *AudioMultiDecoder::getAudio(int64_t usPTS) {
        if (EDecoderStopped >= _decoderState) {
            return nullptr;
        }

        CAudio *pAudio = nullptr;
        do {
            if (_isFinished) {
                //TODO:log warning
                break;
            }
            if (_currentIndex >= _audioDecoders.size()) {
                //TODO:log warning
                break;
            }
            if (usPTS < _targetStartTime || usPTS >= _targetEndTime) {
                break;
            }
            for (; !_isFinished;) {
                if (EDecoderStopped >= _decoderState) {
                    break;
                }
                pAudio = _audioDecoders[_currentIndex]->getAudio(usPTS);
                if (pAudio) {
                    if (0 == _audioBufferSize) {
                        pAudio->m_uTrackId = _pTrack->getId();
                        break;
                    }
                    CAudio *mergedAudio = CAudio::create();
                    mergedAudio->resize(_trunkSize);
                    mergedAudio->m_uSize = _trunkSize;
                    mergedAudio->m_uTrackId = _pTrack->getId();
                    memcpy(mergedAudio->m_pbyVoice, _audioBufferPool, _audioBufferSize);
                    memcpy(mergedAudio->m_pbyVoice + _audioBufferSize, pAudio->m_pbyVoice,
                           _trunkSize - _audioBufferSize);
                    mergedAudio->m_tParams.m_eFormat = AudioDecoder::s_sampleFormat;
                    mergedAudio->m_tParams.m_ullChannelLayout = AudioDecoder::s_channelLayout;
                    mergedAudio->m_tParams.m_eSampleRate = AudioDecoder::s_sampleRate;

                    uint32_t leftSize = pAudio->m_uSize + _audioBufferSize - _trunkSize;
                    memcpy(_audioBufferPool, pAudio->m_pbyVoice + (_trunkSize - _audioBufferSize),
                           leftSize);
                    _audioBufferSize = leftSize;

                    CAudio::release(pAudio);
                    pAudio = mergedAudio;
                    break;
                } else {
                    uint32_t dataSize = 0;
                    uint8_t *dataBuffer = _audioDecoders[_currentIndex]->getRemainderBuffer(
                            dataSize);
                    if (dataBuffer && dataSize > 0) {
                        if (_audioBufferSize + dataSize >= _trunkSize) {
                            pAudio = CAudio::create();
                            pAudio->resize(_trunkSize);
                            pAudio->m_uSize = _trunkSize;
                            pAudio->m_uTrackId = _pTrack->getId();
                            memcpy(pAudio->m_pbyVoice, _audioBufferPool, _audioBufferSize);
                            memcpy(pAudio->m_pbyVoice + _audioBufferSize, dataBuffer,
                                   _trunkSize - _audioBufferSize);
                            pAudio->m_tParams.m_eFormat = AudioDecoder::s_sampleFormat;
                            pAudio->m_tParams.m_ullChannelLayout = AudioDecoder::s_channelLayout;
                            pAudio->m_tParams.m_eSampleRate = AudioDecoder::s_sampleRate;

                            uint32_t leftSize = dataSize + _audioBufferSize - _trunkSize;
                            memcpy(_audioBufferPool, dataBuffer + (_trunkSize - _audioBufferSize),
                                   leftSize);
                            _audioBufferSize = leftSize;
                        } else {
                            memcpy(_audioBufferPool + _audioBufferSize, dataBuffer, dataSize);
                            _audioBufferSize += dataSize;
                        }

                        if (_currentIndex >= _audioDecoders.size() - 1) {
                            if (!pAudio) {
                                if (0 < _audioBufferSize) {
                                    pAudio = CAudio::create();
                                    pAudio->resize(_trunkSize);
                                    pAudio->m_uSize = _trunkSize;
                                    pAudio->m_uTrackId = _pTrack->getId();
                                    memcpy(pAudio->m_pbyVoice, _audioBufferPool, _audioBufferSize);
                                    pAudio->m_tParams.m_eFormat = AV_SAMPLE_FMT_S16;
                                    pAudio->m_tParams.m_ullChannelLayout = AV_CH_LAYOUT_STEREO;
                                    pAudio->m_tParams.m_eSampleRate = EM_SAMPLE_RATE_44_1;

                                    _audioBufferSize = 0;
                                } else {
                                    //TODO:log debug
                                    _isFinished = true;
                                }
                            }
                        } else {
                            ++_currentIndex;
                        }
                        if (pAudio) {
                            break;
                        }
                    } else {
                        if (_currentIndex >= _audioDecoders.size() - 1) {
                            if (0 < _audioBufferSize) {
                                pAudio = CAudio::create();
                                pAudio->resize(_trunkSize);
                                pAudio->m_uSize = _trunkSize;
                                pAudio->m_uTrackId = _pTrack->getId();
                                memcpy(pAudio->m_pbyVoice, _audioBufferPool, _audioBufferSize);
                                pAudio->m_tParams.m_eFormat = AV_SAMPLE_FMT_S16;
                                pAudio->m_tParams.m_ullChannelLayout = AV_CH_LAYOUT_STEREO;
                                pAudio->m_tParams.m_eSampleRate = EM_SAMPLE_RATE_44_1;

                                _audioBufferSize = 0;
                            } else {
                                //TODO:log debug
                                _isFinished = true;
                            }
                        } else {
                            ++_currentIndex;
                        }
                        if (pAudio) {
                            break;
                        }
                    }
                }
            }
            break;
        } while (false);

        return pAudio;
    }

    IDecoder *AudioMultiDecoder::_createSubDecoder(int64_t targetStartTime, ITrack *srcTrack) {
        if (!srcTrack || !srcTrack->isSourceValid()) {
            //TODO:log error
            return nullptr;
        }

        IDecoder *pDecoder = nullptr;
        switch (srcTrack->getSourceType()) {
            case EM_SOURCE_MULTI:
                pDecoder = new AudioMultiDecoder(srcTrack);
                break;
            case EM_SOURCE_FILE:
                pDecoder = new AudioDecoder(srcTrack);
                break;
            case EM_SOURCE_SILENCE:
                pDecoder = new SilenceDecoder(srcTrack);
                break;
            default:
                break;
        }
        if (!pDecoder || !pDecoder->prepare()) {
            //TOOD:log error
            delete pDecoder;
            pDecoder = nullptr;
        } else {
            int64_t targetDuration = pDecoder->getTargetEndTime() - pDecoder->getTargetStartTime();
            pDecoder->setTargetTime(targetStartTime, targetDuration);
        }

        return pDecoder;
    }

    void AudioMultiDecoder::_parseTime() {
        _targetStartTime = _pTrack->getPlayStart() * 1000;
        int64_t dataDuration = _pTrack->getDataDuration();
        int64_t playDuration =
                _pTrack->getPlayDuration() < 0 ? dataDuration : _pTrack->getPlayDuration() * 1000;
        _targetEndTime = _targetStartTime + std::min(dataDuration, playDuration);
    }

}