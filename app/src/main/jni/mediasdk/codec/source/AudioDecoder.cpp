#include <AudioDecoder.h>
#include <thread>
#include <unistd.h>
#include <autolog.h>
#include <autolock.h>
#include <track.h>
#include <constant.h>
#include <sound.h>

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
#else
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#endif

namespace paomiantv {

    AudioDecoder::AudioDecoder(ITrack *pTrack)
            : IDecoder(pTrack), _decoderLock(nullptr), _decoderThread(nullptr), _seekList(),
              _audioSemaphore(nullptr), _pFormatContext(nullptr), _pCodecContext(nullptr),
              _pFrame(nullptr), m_fVolume(pTrack ? ((CTrack *) pTrack)->getVolume() : 1.0f),
              _pRemainderFrame(nullptr), _remainderFrameSize(0), _audioFilter(nullptr),
              _sourceID(0), _audioBufferPool(nullptr), _audioBufferCapacity(0), _audioBufferSize(0),
              _trunkSize(0), _streamIndex(-1), _inputFinish(false), _outputFinish(false) {
        this->initialize();
    }

    AudioDecoder::~AudioDecoder() {
        release();
        reset();
    }

    bool AudioDecoder::initialize() {
        _decoderLock = new CLock();
        _audioList = new CSafeQueue<CAudio>(_cacheCount);
        return true;
    }

    void AudioDecoder::reset() {
        if (_decoderLock) {
            delete _decoderLock;
            _decoderLock = nullptr;
        }
        if (_audioList) {
            _audioList->disable();
            _audioList->clear();
            delete _audioList;
            _audioList = nullptr;
        }
    }

    bool AudioDecoder::prepare() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderInvalid: {
                if (!_validateTrack(EM_TRACK_AUDIO, EM_SOURCE_FILE)) {
                    return false;
                }
                _parseTime();

                _decoderState = EDecoderStopped;
                _audioSemaphore = new CSemaphore(_cacheCount);
                _startDecoderThread();
            }
                break;
            default:
                return false;
        }

        return true;
    }

    void AudioDecoder::release() {
        CAutoLock autoLock(_decoderLock);
        stop();
        _decoderState = EDecoderInvalid;
        if (_decoderThread) {
            _decoderThread->join();
            delete _decoderThread;
            _decoderThread = nullptr;
            if (_audioSemaphore) {
                delete _audioSemaphore;
                _audioSemaphore = nullptr;
            }
        }
    }

    void AudioDecoder::start() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderStopped:
                if (_audioList) {
                    _audioList->disable();
                    _audioList->clear();
                    _audioList->enable();
                }
                _seekList.push_back(_targetStartTime);
                _decoderState = EDecoderPlaying;
                break;
            default:
                break;
        }
    }

    void AudioDecoder::stop() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderPlaying:
            case EDecoderPaused: {
                _decoderState = EDecoderStopped;
                _flush();
                if (_audioList) {
                    _audioList->disable();
                }
            }
                break;
            default:
                break;
        }
    }

    void AudioDecoder::pause() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderPlaying:
                _decoderState = EDecoderPaused;
                break;
            default:
                break;
        }
    }

    void AudioDecoder::resume() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderPaused:
                _decoderState = EDecoderPlaying;
                break;
            default:
                break;
        }
    }

    void AudioDecoder::seekTo(int64_t usTime) {
        CAutoLock autoLock(_decoderLock);
        if (EDecoderInvalid == _decoderState) {
            return;
        }
        _audioList->disable();
        _seekList.push_back(usTime);
    }

    void AudioDecoder::flush() {
        //Reserved.
    }

    CVLayerParam *AudioDecoder::getImageLayer(int64_t usPTS) {
        //Log warning
        return nullptr;
    }

    CAudio *AudioDecoder::getAudio(int64_t usPTS) {
        if (EDecoderInvalid == _decoderState) {
            return nullptr;
        }
        if (usPTS < _targetStartTime || usPTS >= _targetEndTime) {
            return nullptr;
        }

        CAudio *pAudio = nullptr;
#if 1
        if (!_outputFinish) {
            _audioList->pop(pAudio);
            if (!pAudio) {
                while (!_seekList.empty()) {
                    usleep(3000);
                }
            }
        } else {
            if (0 < _audioList->size()) {
                _audioList->pop(pAudio);
            } else {
                pAudio = CAudio::create();
                pAudio->resize(_trunkSize);
                pAudio->m_uSize = _trunkSize;
                pAudio->m_uTrackId = _pTrack->getId();
                pAudio->m_tParams.m_eFormat = s_sampleFormat;
                pAudio->m_tParams.m_ullChannelLayout = s_channelLayout;
                pAudio->m_tParams.m_eSampleRate = s_sampleRate;
            }
        }
#else
        if (!_outputFinish) {
            while (EDecoderStopped < _decoderState) {
                if (!_audioList.empty()) {
                    if (usPTS < _targetStartTime || !_seekList.empty()) {
                        break;
                    }
                    else if (_targetEndTime >= usPTS) {
                        pAudio = _audioList.front();
                        _audioList.pop_front();
                        _audioSemaphore->post();
                    } else {
                        _flush();
                        break;
                    }
                    break;
                } else {
                    usleep(3000);
                }
            }
        } else {
            if (!_audioList.empty() && _seekList.empty()) {
                if (usPTS >= _targetStartTime && usPTS <= _targetEndTime) {
                    pAudio = _audioList.front();
                    _audioList.pop_front();
                    _audioSemaphore->post();
                }
                else if (usPTS > _targetEndTime) {
                    _flush();
                }
            } else {
                if (usPTS <= _targetEndTime) {
                    pAudio = CAudio::create();
                    pAudio->resize(_trunkSize);
                    pAudio->m_uSize = _trunkSize;
                    pAudio->m_uTrackId = _pTrack->getId();
                    pAudio->m_tParams.m_eFormat = s_sampleFormat;
                    pAudio->m_tParams.m_ullChannelLayout = s_channelLayout;
                    pAudio->m_tParams.m_eSampleRate = s_sampleRate;
                }
            }
        }
#endif
        return pAudio;
    }

    uint8_t *AudioDecoder::getRemainderBuffer(uint32_t &size) {
        if (_inputFinish) {
            size = _audioBufferSize;
            return _audioBufferPool;
        } else {
            size = 0;
            return nullptr;
        }
    }

    void AudioDecoder::run() {
        if (!_prepare()) {
            _release();
            return;
        }

        while (EDecoderInvalid < _decoderState) {
            if (!_seekList.empty()) {
                _processCommand();
            }
            if (!_outputFinish && EDecoderStopped < _decoderState) {
                do {
                    if (!_decodeFrame()) {
                        break;
                    }
                    if (_processBuffer()) {
                        _writeBuffer();
                    }
                    av_frame_unref(_pFrame);
                } while (false);

                if (_outputFinish) {
                    if (_isLoop) {
                        _seekTo(_sourceStartTime);
                    }
                }
            } else {
                usleep(2000);
            }
        }

        _release();
    }

    bool AudioDecoder::_prepare() {
        if (!_initDecoder()) {
            return false;
        }
        if (!_initFrames()) {
            return false;
        }
        if (!_initAudioFilter()) {
            return false;
        }
        if (!_initBuffer()) {
            return false;
        }
        return true;
    }

    void AudioDecoder::_release() {
        if (_audioFilter) {
            _audioFilter->destroy();
            _audioFilter = nullptr;
        }
        if (_pFrame) {
            av_frame_free(&_pFrame);
            _pFrame = nullptr;
        }
        if (_pRemainderFrame) {
            av_frame_free(&_pRemainderFrame);
            _pRemainderFrame = nullptr;
        }
        if (_pCodecContext) {
            avcodec_free_context(&_pCodecContext);
            _pCodecContext = nullptr;
        }
        if (_pFormatContext) {
            avformat_close_input(&_pFormatContext);
            _pFormatContext = nullptr;
        }

        _streamIndex = 0;
        _trunkSize = 0;
        _audioBufferCapacity = 0;
        _audioBufferSize = 0;
        if (_audioBufferPool) {
            free(_audioBufferPool);
            _audioBufferPool = nullptr;
        }
    }

    static void _threadFunc(AudioDecoder *pDecoder) {
        if (pDecoder) {
            pDecoder->run();
        }
    }

    void AudioDecoder::_startDecoderThread() {
        _decoderThread = new std::thread(_threadFunc, this);
    }

    static int _openCodecContext(
            AVFormatContext *formatContext, /*in*/
            enum AVMediaType mediaType, /*in*/
            int *streamIndex, /*out*/
            AVCodecContext **codecContext /*out*/) {
        if (!formatContext || !streamIndex || !codecContext) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._openCodecContext:Context is null!");
            return -1;
        }

        int errorCode = 0;
        errorCode = av_find_best_stream(formatContext, mediaType, -1, -1, nullptr, 0);
        if (errorCode < 0) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._openCodecContext:Failed to find stream!");
            return errorCode;
        } else {
            int curIndex = errorCode;
            AVStream *pStream = formatContext->streams[curIndex];
            AVCodec *pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
            if (!pCodec) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "AudioDecoder._openCodecContext:Failed to find codec!");
                return AVERROR(EINVAL);
            }
            *codecContext = avcodec_alloc_context3(pCodec);
            if (!(*codecContext)) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "AudioDecoder._openCodecContext:Failed to alloc codec context!");
                return AVERROR(ENOMEM);
            }
            errorCode = avcodec_parameters_to_context(*codecContext, pStream->codecpar);
            if (0 > errorCode) {
                avcodec_free_context(codecContext);
                *codecContext = nullptr;
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "AudioDecoder._openCodecContext:Failed to set param!");
                return errorCode;
            }
            errorCode = avcodec_open2(*codecContext, pCodec, nullptr);
            if (0 > errorCode) {
                avcodec_free_context(codecContext);
                *codecContext = nullptr;
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "AudioDecoder._openCodecContext:Failed to open codec!");
                return errorCode;
            }
            *streamIndex = curIndex;
        }

        return 0;
    }

    bool AudioDecoder::_initDecoder() {
        std::string errorInfo = "";
        do {
            int errorCode = 0;
            uint8_t *path = nullptr;
            uint32_t size = 0;
            _pTrack->getSourceData(path, size);
            if (path == nullptr || size == 0) {
                errorInfo = "Track source is invalid!";
                break;
            }
            errorCode = avformat_open_input(
                    &_pFormatContext,
                    (char *) path,
                    nullptr, nullptr);
            if (0 > errorCode) {
                errorInfo = "Failed to open input file!";
                _pFormatContext = nullptr;
                break;
            }
            errorCode = avformat_find_stream_info(_pFormatContext, nullptr);
            if (0 > errorCode) {
                errorInfo = "Failed to find stream info!";
                break;
            }
            errorCode = _openCodecContext(
                    _pFormatContext, AVMEDIA_TYPE_AUDIO,
                    &_streamIndex, &_pCodecContext);
            if (0 > errorCode) {
                errorInfo = "Failed to open codec!";
                break;
            }
        } while (false);

        if (!errorInfo.empty()) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "AudioDecoder.initDecoder:%s",
                                errorInfo.c_str());
            return false;
        }
        return true;
    }

    bool AudioDecoder::_initFrames() {
        std::string errorInfo = "";

        do {
            _pFrame = av_frame_alloc();
            _pRemainderFrame = av_frame_alloc();
            if (!_pFrame || !_pRemainderFrame) {
                errorInfo = "Failed to allocate frame for decode!";
                break;
            }

            _pRemainderFrame->sample_rate = _pCodecContext->sample_rate;
            _pRemainderFrame->format = _pCodecContext->sample_fmt;
            _pRemainderFrame->channel_layout = _pCodecContext->channel_layout;
            _pRemainderFrame->channels = av_get_channel_layout_nb_channels(
                    _pCodecContext->channel_layout);
            _pRemainderFrame->nb_samples = AUDIO_SAMPLE_COUNT_PER_FRAME;

            if (0 > av_frame_get_buffer(_pRemainderFrame, 0)) {
                errorInfo = "Failed to get buffer for remainder frame!";
                break;
            }
        } while (false);

        if (!errorInfo.empty()) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "AudioDecoder.createFrames:%s",
                                errorInfo.c_str());
            return false;
        }
        return true;
    }

    bool AudioDecoder::_initAudioFilter() {
        _audioFilter = CAFilterComplex::Create(s_sampleRate, s_sampleFormat, s_channelLayout);
        if (!_audioFilter) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._initAudioFilter:Failed to create audio filter!");
            return false;
        }
        BOOL32 ret = _audioFilter->addInput(
                (EMSampleRate) _pCodecContext->sample_rate,
                _pCodecContext->sample_fmt,
                _pCodecContext->channel_layout,
                _pTrack->getWeight(),
                _sourceID);
        if (!ret) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._initAudioFilter:Failed to add input!");
            return false;
        }
        ret = _audioFilter->addVolumeInSource(_sourceID, ((CTrack *) _pTrack)->getVolume());
        if (!ret) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._initAudioFilter:Failed to add volume!");
            return false;
        }
        ret = _audioFilter->addFormatInSource(
                _sourceID, EM_SAMPLE_RATE_44_1,
                AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO);
        if (!ret) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._initAudioFilter:Failed to add format!");
            return false;
        }
        int errorCode = _audioFilter->configure();
        if (0 > errorCode) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._initAudioFilter:Failed to configure audioFilter!");
            return false;
        }
        return true;
    }

    bool AudioDecoder::_initBuffer() {
        _trunkSize = AUDIO_SAMPLE_COUNT_PER_FRAME * (0x0001) << s_sampleFormat;
        _trunkSize *= (uint32_t) av_get_channel_layout_nb_channels(s_channelLayout);
        _audioBufferCapacity = _trunkSize * BLOCK_QUEUE_SIZE;
        _audioBufferPool = (uint8_t *) malloc(_audioBufferCapacity);
        _audioBufferSize = 0;
        return true;
    }

    void AudioDecoder::_initTime() {
        int64_t dataDuration = ((CTrack *) _pTrack)->getDataDuration();
        _sourceStartTime = ((CTrack *) _pTrack)->getCutStart() * 1000;
        int64_t cutDuration = ((CTrack *) _pTrack)->getCutDuration();
        _sourceEndTime = cutDuration > 0 ? (_sourceStartTime + cutDuration) : dataDuration;
        _sourceEndTime = std::min(dataDuration, _sourceEndTime);

        _targetStartTime = ((CTrack *) _pTrack)->getPlayStart() * 1000;
        int64_t targetDuration =
                _pTrack->getPlayDuration() < 0 ? dataDuration : _pTrack->getPlayDuration() * 1000;
        _targetEndTime = _targetStartTime + targetDuration;
    }

    bool AudioDecoder::_adjustSeekTimestamp(int64_t time, int64_t &out) {
        return false;
    }

    bool AudioDecoder::_readPacket() {
        if (_inputFinish) {
            return true;
        }

        AVPacket inputPacket;
        av_init_packet(&inputPacket);
        inputPacket.data = nullptr;
        inputPacket.size = 0;

        int errorCode = av_read_frame(_pFormatContext, &inputPacket);
        while (0 == errorCode) {
            if (_streamIndex != inputPacket.stream_index) {
                av_packet_unref(&inputPacket);
                av_init_packet(&inputPacket);
                inputPacket.data = nullptr;
                inputPacket.size = 0;
                errorCode = av_read_frame(_pFormatContext, &inputPacket);
            } else {
                break;
            }
        }

        if (AVERROR_EOF == errorCode) {
            _inputFinish = true;
        } else if (0 != errorCode) {
            av_packet_unref(&inputPacket);
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._readPacket:Failed to read packet!");
            return false;
        }

        errorCode = avcodec_send_packet(_pCodecContext, &inputPacket);
        if (0 > errorCode) {
            av_packet_unref(&inputPacket);
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._readPacket:Failed to send packet!");
            return false;
        }

        av_packet_unref(&inputPacket);
        return true;
    }

    bool AudioDecoder::_decodeFrame() {
        if (_outputFinish) {
            return false;
        }

        if (!_readPacket()) {
            return false;
        }
        int errorCode = avcodec_receive_frame(_pCodecContext, _pFrame);
        if (0 == errorCode) {
            int64_t usPTS = av_rescale_q(
                    _pFrame->pts,
                    _pFormatContext->streams[_streamIndex]->time_base,
                    AV_TIME_BASE_Q);
            if (usPTS > _sourceEndTime) {
                av_frame_unref(_pFrame);
                _outputFinish = true;
                return false;
            }
            return true;
        } else if (AVERROR(EAGAIN) == errorCode) {
            //more data
        } else if (AVERROR_EOF == errorCode) {
            _outputFinish = true;
        } else {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._decodeFrame:Failed to receive frame:%d", errorCode);
        }
        return false;
    }

    bool AudioDecoder::_processBuffer() {
        if (round(m_fVolume * 100) != round(((CTrack *) _pTrack)->getVolume() * 100)) {
            m_fVolume = ((CTrack *) _pTrack)->getVolume();
            char tmp[64];
            sprintf(tmp, "%.4f", m_fVolume);
            avfilter_graph_send_command(_audioFilter->getGraph()->getGraph(),
                                        _audioFilter->getInputVolume(_sourceID)->getName(),
                                        "volume", tmp, NULL, 0, 0);
        }
        if (_remainderFrameSize == 0 &&
            _pFrame->nb_samples == AUDIO_SAMPLE_COUNT_PER_FRAME) {
            u8 *tmp = NULL;
            int tmps = 0;

            _audioFilter->process(1, &_sourceID, &_pFrame, tmp, tmps);
            if (tmp != NULL && tmps > 0) {
                if (_audioBufferSize + tmps > _audioBufferCapacity) {
                    u8 *p = (u8 *) realloc(_audioBufferPool, _audioBufferSize + tmps);
                    if (p == NULL) {
                        __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                            "AudioDecoder._processBuffer.Failed to realloc mem!");
                        return false;
                    } else {
                        _audioBufferPool = p;
                        _audioBufferCapacity = _audioBufferSize + tmps;
                    }
                }
                memcpy(_audioBufferPool + _audioBufferSize, tmp, (u32) tmps);
                _audioBufferSize += tmps;
            }
        } else {
            // handle the processed data
            u32 sum = _pFrame->nb_samples + _remainderFrameSize;
            u32 count = sum / AUDIO_SAMPLE_COUNT_PER_FRAME;
            u32 rmd = sum % AUDIO_SAMPLE_COUNT_PER_FRAME;
            AVFrame *frame = av_frame_alloc();
            u32 outOffset = 0;
            for (u32 i = 0; i < count; i++) {
                frame->sample_rate = _pFrame->sample_rate;
                frame->format = _pFrame->format;
                frame->channel_layout = _pFrame->channel_layout;
                frame->nb_samples = AUDIO_SAMPLE_COUNT_PER_FRAME;
                if (av_frame_get_buffer(frame, 0) < 0) {
                    __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                        "AudioDecoder._processBuffer.Failed to get buffer for frame!");
                    return false;
                }
                if (_remainderFrameSize > 0) {
                    av_samples_copy(frame->extended_data,
                                    _pRemainderFrame->extended_data,
                                    0,
                                    0,
                                    _remainderFrameSize,
                                    _pRemainderFrame->channels,
                                    static_cast<AVSampleFormat>(_pRemainderFrame->format));
                    av_samples_copy(frame->extended_data,
                                    _pFrame->extended_data,
                                    _remainderFrameSize,
                                    outOffset,
                                    AUDIO_SAMPLE_COUNT_PER_FRAME - _remainderFrameSize,
                                    _pFrame->channels,
                                    (AVSampleFormat) _pFrame->format);
                    outOffset += (AUDIO_SAMPLE_COUNT_PER_FRAME - _remainderFrameSize);
                    _remainderFrameSize = 0;
                } else {
                    av_samples_copy(frame->extended_data, _pFrame->extended_data,
                                    0,
                                    outOffset,
                                    AUDIO_SAMPLE_COUNT_PER_FRAME,
                                    _pFrame->channels,
                                    (AVSampleFormat) _pFrame->format);
                    outOffset += AUDIO_SAMPLE_COUNT_PER_FRAME;
                }

                u8 *tmp = NULL;
                int tmps = 0;
                _audioFilter->process(1, &_sourceID, &frame, tmp, tmps);
                if (tmp != NULL && tmps > 0) {
                    if (_audioBufferSize + tmps > _audioBufferCapacity) {
                        u8 *p = (u8 *) realloc(_audioBufferPool, _audioBufferSize + tmps);
                        if (p == NULL) {
                            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                                "AudioDecoder._processBuffer.Failed to realloc mem!");
                            return false;
                        } else {
                            _audioBufferPool = p;
                            _audioBufferCapacity = _audioBufferSize + tmps;
                        }
                    }
                    memcpy(_audioBufferPool + _audioBufferSize, tmp, (u32) tmps);
                    _audioBufferSize += tmps;
                }
            }

#ifdef LOG_ENABLE
            if (rmd != _pFrame->nb_samples - outOffset + _remainderFrameSize) {
                LOGE("this situation will never happend!");
            }
#endif
            av_samples_copy(_pRemainderFrame->extended_data, _pFrame->extended_data,
                            _remainderFrameSize,
                            outOffset,
                            _pFrame->nb_samples - outOffset,
                            _pFrame->channels,
                            static_cast<AVSampleFormat>(_pFrame->format));
            _remainderFrameSize += (_pFrame->nb_samples - outOffset);
        }
        return true;
    }

    bool AudioDecoder::_writeBuffer() {
        if (_audioBufferSize > 0) {
            // handle the processed data
            u32 count = _audioBufferSize / _trunkSize;
            u32 rmd = _audioBufferSize % _trunkSize;

            u32 outOffset = 0;
            for (u32 i = 0; i < count; i++) {
                CAudio *audio = CAudio::create();
                audio->resize(_trunkSize);
                memcpy(audio->m_pbyVoice, _audioBufferPool + outOffset, _trunkSize);
                outOffset += _trunkSize;
                audio->m_uSize = _trunkSize;
                audio->m_uTrackId = _pTrack->getId();
                audio->m_tParams.m_eFormat = s_sampleFormat;
                audio->m_tParams.m_ullChannelLayout = s_channelLayout;
                audio->m_tParams.m_eSampleRate = s_sampleRate;
                if (!_audioList->push(audio)) {
                    CAudio::release(audio);
                }
            }
#ifdef LOG_ENABLE
            if (rmd != _audioBufferSize - outOffset) {
                LOGE("this situation will never happened!");
            }
#endif
            if (count > 0) {
                memmove(_audioBufferPool, _audioBufferPool + outOffset, rmd);
            }
            _audioBufferSize = rmd;
            return true;
        }
        return false;
    }

    void AudioDecoder::_processCommand() {
        CAutoLock autoLock(_decoderLock);
        int64_t usTime = _seekList.back();
        usTime = usTime - _targetStartTime + _sourceStartTime;

        _flush();
        _seekTo(usTime);

        _seekList.clear();
    }

    void AudioDecoder::_seekTo(int64_t usTime) {
        if (usTime < 0) {
            //Log warning.
            usTime = 0;
        }
        if (usTime > _sourceEndTime) {
            __android_log_print(ANDROID_LOG_WARN, "PaomianTV",
                                "AudioDecoder._seekTo:seekTime %lld is invalid!", usTime);
            return;
        }

        avcodec_flush_buffers(_pCodecContext);
        int errorCode = 0;
        errorCode = av_seek_frame(_pFormatContext, -1, usTime, AVSEEK_FLAG_BACKWARD);
        if (errorCode < 0) {
            errorCode = av_seek_frame(_pFormatContext, _streamIndex, usTime, AVSEEK_FLAG_ANY);
        }
        if (errorCode < 0) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "AudioDecoder._seekTo:Failed to seek frame!");
            return;
        }
        _inputFinish = false;
        _outputFinish = false;

        _skipFrame(usTime);
    }

    void AudioDecoder::_skipFrame(int64_t usTime) {
        if (0 >= usTime) {
            return;
        }

        do {
            if (_outputFinish) {
                break;
            }
            if (!_readPacket()) {
                break;
            }

            int errorCode = avcodec_receive_frame(_pCodecContext, _pFrame);
            if (0 == errorCode) {
                int64_t framePTS = av_rescale_q(
                        _pFrame->pts,
                        _pFormatContext->streams[_streamIndex]->time_base,
                        AV_TIME_BASE_Q);

                if (framePTS > (usTime - 40 * 1000) || framePTS > _sourceEndTime) {
                    _processBuffer();
                    _writeBuffer();
                    break;
                }
                av_frame_unref(_pFrame);
            } else if (AVERROR(EAGAIN) == errorCode) {
                //more data
            } else if (AVERROR_EOF == errorCode) {
                _outputFinish = true;
            } else {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "AudioDecoder._skipFrame:Failed to receive frame!");
                break;
            }
        } while (true);
    }

    void AudioDecoder::_flush() {
        _audioList->disable();
        _audioList->clear();
        _audioList->enable();
    }

}
