//
// Created by ASUS on 2018/5/16.
//

#include <autolock.h>
#include <sound.h>
#include <track.h>
#include <AudioDecoder.h>
#include "SilenceDecoder.h"

namespace paomiantv {

    /**
     * Default Target Duration in microseconds.
     */
    static int64_t s_defaultTargetDuration = 1 * 1000 * 1000;

    SilenceDecoder::SilenceDecoder(ITrack *pTrack)
            : IDecoder(pTrack),
              _isPrepared(false) {
        initialize();
    }

    SilenceDecoder::~SilenceDecoder() {
        reset();
    }

    bool SilenceDecoder::prepare() {
        CAutoLock autoLock(_decoderLock);
        if (_isPrepared) {
            return false;
        }

        do {
            if (!_pTrack) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "SilenceDecoder.prepare:Null Track!");
                break;
            }
            if (EM_TRACK_AUDIO != _pTrack->getType()) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "SilenceDecoder.prepare:Invalid track type!");
                break;
            }
            if (EM_SOURCE_SILENCE != _pTrack->getSourceType()) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "SilenceDecoder.prepare:Invalid source type!");
                break;
            }
#if 1
            _parseTime();
#else
            _targetStartTime = _pTrack->getPlayStart() * 1000;
            int64_t dataDuration = _pTrack->getDataDuration() * 1000;
            if (_pTrack->getPlayDuration() < 0) {
                _targetEndTime = LLONG_MAX;
            } else {
                _targetEndTime = std::min(_pTrack->getPlayDuration() * 1000 + _targetStartTime,
                                          dataDuration);
            }
#endif

            _trunkSize = AUDIO_SAMPLE_COUNT_PER_FRAME * (0x0001) << AudioDecoder::s_sampleFormat;
            _trunkSize *= (uint32_t) av_get_channel_layout_nb_channels(
                    AudioDecoder::s_channelLayout);

            _isPrepared = true;
        } while (false);
        return _isPrepared;
    }

    void SilenceDecoder::release() {
        CAutoLock autoLock(_decoderLock);
        if (!_isPrepared) {
            return;
        }
        _isPrepared = false;
        _targetStartTime = 0;
        _targetEndTime = 0;
    }

    void SilenceDecoder::start() {
    }

    void SilenceDecoder::stop() {
    }

    void SilenceDecoder::pause() {
    }

    void SilenceDecoder::resume() {
    }

    void SilenceDecoder::seekTo(int64_t usTime) {
    }

    void SilenceDecoder::flush() {
    }

    CVLayerParam *SilenceDecoder::getImageLayer(int64_t usPTS) {
        return nullptr;
    }

    CAudio *SilenceDecoder::getAudio(int64_t usPTS) {
        if (!_isPrepared) {
            return nullptr;
        }
        if (usPTS < _targetStartTime) {
            return nullptr;
        }
        if (usPTS > _targetEndTime) {
            return nullptr;
        }
        return _createSilenceAudio();
    }

    bool SilenceDecoder::initialize() {
        _decoderLock = new CLock();
    }

    void SilenceDecoder::reset() {
        if (_decoderLock) {
            delete _decoderLock;
            _decoderLock = nullptr;
        }
    }

    CAudio *SilenceDecoder::_createSilenceAudio() {
        CAudio *pAudio = CAudio::create();
        pAudio->resize(_trunkSize);
        pAudio->m_uSize = _trunkSize;
        pAudio->m_uTrackId = _pTrack->getId();
        pAudio->m_tParams.m_eFormat = AudioDecoder::s_sampleFormat;
        pAudio->m_tParams.m_ullChannelLayout = AudioDecoder::s_channelLayout;
        pAudio->m_tParams.m_eSampleRate = AudioDecoder::s_sampleRate;
        return pAudio;
    }

    void SilenceDecoder::_parseTime() {
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