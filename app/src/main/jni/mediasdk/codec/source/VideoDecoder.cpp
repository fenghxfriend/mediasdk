#include <VideoDecoder.h>
#include <autolog.h>
#include <string>
#include <thread>
#include <autolock.h>
#include <track.h>
#include <image.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
#endif

namespace paomiantv {

    /**
     * PTS误差时间，单位为微秒
     */
    static const int64_t s_ptsBiasTime = 1 * 1000;

    /**
     * 默认帧持续时间，单位为微秒
     */
    static const int64_t s_defaultFrameDuration = 40 * 1000;

    VideoDecoder::VideoDecoder(ITrack *pTrack)
            : IDecoder(pTrack), _decoderLock(nullptr), _decoderThread(nullptr), _seekList(),
              _layerList(nullptr), _currentLayer(nullptr), _cacheLayer(nullptr),
              _layerSemaphore(nullptr), _pFormatContext(nullptr), _pCodecContext(nullptr),
              _prevFrameTime(-_minInterval - 1), _pixelFormat(EM_PIXEL_FORMAT_I420),
              _pFrame(nullptr), _formatConverter(nullptr), _streamIndex(0), _frameWidth(0),
              _frameHeight(0), _inputFinish(false), _outputFinish(false) {
        this->initialize();
    }

    VideoDecoder::~VideoDecoder() {
        release();
        reset();
    }

    bool VideoDecoder::initialize() {
        _decoderLock = new CLock();
        _layerList = new CSafeQueue<CVLayerParam>(_cacheCount);
        return true;
    }

    void VideoDecoder::reset() {
        this->release();
        if (_decoderLock) {
            delete _decoderLock;
            _decoderLock = nullptr;
        }
        _seekList.clear();
        if (_layerList) {
            _layerList->disable();
            _layerList->clear();
            _layerList->enable();
            delete _layerList;
            _layerList = nullptr;
        }

        _inputFinish = false;
        _outputFinish = false;
    }

    bool VideoDecoder::prepare() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderInvalid: {
                if (!_validateTrack(EM_TRACK_VIDEO, EM_SOURCE_FILE)) {
                    return false;
                }
                _parseTime();

                _decoderState = EDecoderStopped;
                _layerSemaphore = new CSemaphore(_cacheCount);
                _prevFrameTime = -_minInterval - 1;
                _startDecoderThread();
            }
                break;
            default:
                return false;
        }

        return true;
    }

    void VideoDecoder::release() {
        CAutoLock autoLock(_decoderLock);
        stop();
        _decoderState = EDecoderInvalid;
        if (_decoderThread) {
            _decoderThread->join();
            delete _decoderThread;
            _decoderThread = nullptr;
            if (_layerSemaphore) {
                delete _layerSemaphore;
                _layerSemaphore = nullptr;
            }
            _seekList.clear();
        }
        if (_currentLayer) {
            CVLayerParam::release(_currentLayer);
            _currentLayer = nullptr;
        }
        if (_cacheLayer) {
            CVLayerParam::release(_cacheLayer);
            _cacheLayer = nullptr;
        }
    }

    void VideoDecoder::start() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderStopped:
                if (_layerList) {
                    _layerList->disable();
                    _layerList->clear();
                    _layerList->enable();
                }
                _seekList.push_back(_targetStartTime);
                _decoderState = EDecoderPlaying;
                break;
            default:
                break;
        }
    }

    void VideoDecoder::stop() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderPlaying:
            case EDecoderPaused: {
                _decoderState = EDecoderStopped;
                _flush();
                if (_layerList) {
                    _layerList->disable();
                }
            }
                break;
            default:
                break;
        }
    }

    void VideoDecoder::pause() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderPlaying:
                _decoderState = EDecoderPaused;
                break;
            default:
                break;
        }
    }

    void VideoDecoder::resume() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderPaused:
                _decoderState = EDecoderPlaying;
                break;
            default:
                break;
        }
    }

    void VideoDecoder::seekTo(int64_t usTime) {
        CAutoLock autoLock(_decoderLock);
        if (EDecoderInvalid == _decoderState) {
            return;
        }
        _layerList->disable();
        _seekList.push_back(usTime);
    }

    void VideoDecoder::flush() {
        CAutoLock autoLock(_decoderLock);
    }

#if 1

    CVLayerParam *VideoDecoder::getImageLayer(int64_t usPTS) {
        if (EDecoderInvalid == _decoderState) {
            return nullptr;
        }

        if (usPTS < _targetStartTime && !_showFirstFrame) {
            return nullptr;
        }
        if (usPTS > _targetEndTime) {
            if (!_showLastFrame) {
                return nullptr;
            }
            usPTS = _targetEndTime - 5 * 1000;
        }
        CVLayerParam *pLayer = nullptr;
        do {
            if (!_seekList.empty()) {
                usleep(3000);
                continue;
            }

            if (_checkLayerTime(_currentLayer, usPTS)) {
                pLayer = _currentLayer;
                break;
            }
            if (_currentLayer) {
                CVLayerParam::release(_currentLayer);
                _currentLayer = nullptr;
            }
            if (_outputFinish && _layerList->size() == 0) {
                break;
            }
            _layerList->pop(pLayer);
            if (!pLayer) {
                break;
            }
            if (!_checkLayerTime(pLayer, usPTS)) {
                CVLayerParam::release(pLayer);
                pLayer = nullptr;
                continue;
            }
            _currentLayer = pLayer;
            break;
        } while (EDecoderInvalid < _decoderState);
        if (pLayer) {
            pLayer = pLayer->clone();
            _currentTime = pLayer->m_sllTimeStampUS;
        }
        return pLayer;
    }

#else
    CVLayerParam *VideoDecoder::getImageLayer(int64_t usPTS)
    {
        CAutoLock autoLock (_decoderLock);
        if (usPTS < _targetStartTime || usPTS > _targetEndTime) {
            return nullptr;
        }

        CVLayerParam *pLayer = nullptr;
        while(_layerList.empty()) {
            pLayer = _layerList.front();
            if (!pLayer) {
                _layerList.pop_front();
                _layerSemaphore->post();
                continue;
            }
            if (pLayer->m_sllTimeStampUS >= (usPTS - s_ptsBiasTime)) {
                break;
            } else {
                if (_layerList.empty()) {
                    break;
                } else {
                    _layerList.pop_front();
                    _layerSemaphore->post();
                    CVLayerParam::release(pLayer);
                    pLayer = nullptr;
                }
            }
        }
        if (pLayer) {
            pLayer = pLayer->clone();
            _currentTime = pLayer->m_sllTimeStampUS;
        }
        return pLayer;
    }
#endif

    CAudio *VideoDecoder::getAudio(int64_t usPTS) {
        LOGD("No audio for VideoDecoder!");
        return nullptr;
    }

    void VideoDecoder::run() {
        if (!_prepare()) {
            return;
        }

        while (EDecoderInvalid < _decoderState) {
            if (!_seekList.empty()) {
                _processCommand();
            }
            if (!_outputFinish && EDecoderStopped < _decoderState) {
                if (_decodeFrame()) {
                    _cacheFrame();
                }

                if (_outputFinish) {
                    if (_isLoop) {
                        _seekTo(_sourceStartTime);
                    }
                }
            } else {
                usleep(2000);
            }
        }

        _releaseDecoder();
    }

    static void _threadFunc(VideoDecoder *pDecoder) {
        if (pDecoder) {
            pDecoder->run();
        }
    }

    void VideoDecoder::_startDecoderThread() {
        _decoderThread = new std::thread(_threadFunc, this);
    }

    bool VideoDecoder::_prepare() {
        if (_createDecoder()) {
            if (_sourceStartTime > 0) {
                _seekTo(_sourceStartTime, true);
            }
            return true;
        }

        _releaseDecoder();
        _decoderState = EDecoderInvalid;
        return false;
    }

    static int _openCodecContext(
            AVFormatContext *formatContext, /*in*/
            enum AVMediaType mediaType, /*in*/
            int *streamIndex, /*out*/
            AVCodecContext **codecContext /*out*/) {
        if (!formatContext || !streamIndex || !codecContext) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "VideoDecoder._openCodecContext:Context is null!");
            return -1;
        }

        int errorCode = 0;
        errorCode = av_find_best_stream(formatContext, mediaType, -1, -1, nullptr, 0);
        if (errorCode < 0) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "VideoDecoder._openCodecContext:Failed to find stream!");
            return errorCode;
        } else {
            int curIndex = errorCode;
            AVStream *pStream = formatContext->streams[curIndex];
            AVCodec *pCodec = nullptr;
            switch (pStream->codecpar->codec_id) {
#if HW_MEDIACODEC_ENABLE
                case AV_CODEC_ID_H264: {
                    pCodec = avcodec_find_decoder_by_name("h264_mediacodec");
                    break;
                }
                case AV_CODEC_ID_MPEG4: {
                    pCodec = avcodec_find_decoder_by_name("mpeg4_mediacodec");
                    break;
                }
#endif
                default:
                    pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
                    break;
            }
            if (!pCodec) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "VideoDecoder._openCodecContext:find hardware codec failed, find a software codec!");
                pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
            }

            if (!pCodec) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "VideoDecoder._openCodecContext:Failed to find codec!");
                return AVERROR(EINVAL);
            }
            *codecContext = avcodec_alloc_context3(pCodec);
            if (!(*codecContext)) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "VideoDecoder._openCodecContext:Failed to alloc codec context!");
                return AVERROR(ENOMEM);
            }
            errorCode = avcodec_parameters_to_context(*codecContext, pStream->codecpar);
            if (0 > errorCode) {
                avcodec_free_context(codecContext);
                *codecContext = nullptr;
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "VideoDecoder._openCodecContext:Failed to set param!");
                return errorCode;
            }
            errorCode = avcodec_open2(*codecContext, pCodec, nullptr);
            if (0 > errorCode) {
                avcodec_free_context(codecContext);
                *codecContext = nullptr;
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "VideoDecoder._openCodecContext:Failed to open codec!");
                return errorCode;
            }
            *streamIndex = curIndex;
        }

        return 0;
    }

    bool VideoDecoder::_createDecoder() {
        if (!_validateTrack(EM_TRACK_VIDEO, EM_SOURCE_FILE)) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "VideoDecoder._createDecoder:Invalid track!");
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "***---pTrack = %p, sourceType = %d",
                                _pTrack,
                                _pTrack ? _pTrack->getSourceType() : -1);
            return false;
        }

        std::string errorInfo = "";
        do {
            int errorCode = 0;
            uint8_t *data = nullptr;
            uint32_t size = 0;
            _pTrack->getSourceData(data, size);
            errorCode = avformat_open_input(
                    &_pFormatContext,
                    (char *) data,
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
                    _pFormatContext, AVMEDIA_TYPE_VIDEO,
                    &_streamIndex, &_pCodecContext);
            if (0 > errorCode) {
                errorInfo = "Failed to open codec!";
                break;
            }
            _formatConverter = sws_getContext(_pCodecContext->width,
                                              _pCodecContext->height,
                                              _pCodecContext->pix_fmt,
                                              _pCodecContext->width,
                                              _pCodecContext->height,
                                              AV_PIX_FMT_YUV420P, 0,
                                              nullptr, nullptr, nullptr);
            _frameWidth = _pCodecContext->width;
            _frameHeight = _pCodecContext->height;
            _pixelFormat = EM_PIXEL_FORMAT_I420;

            _pFrame = av_frame_alloc();
            if (!_pFrame) {
                errorInfo = "Failed to allocate frame for decode!";
                break;
            }
        } while (false);

        if (!errorInfo.empty()) {
            _releaseDecoder();
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "VideoDecoder.createDecoder:%s",
                                errorInfo.c_str());
            return false;
        }

        return true;
    }

    void VideoDecoder::_releaseDecoder() {
        if (_pCodecContext) {
            avcodec_free_context(&_pCodecContext);
            _pCodecContext = nullptr;
        }
        if (_pFormatContext) {
            avformat_close_input(&_pFormatContext);
            _pFormatContext = nullptr;
        }
        if (_pFrame) {
            av_frame_free(&_pFrame);
            _pFrame = nullptr;
        }
        _streamIndex = 0;
        _frameWidth = 0;
        _frameHeight = 0;
        _pixelFormat = EM_PIXEL_FORMAT_I420;
    }

    bool VideoDecoder::_decodeFrame() {
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
            if (!_minInterval > (usPTS - _prevFrameTime)) {
                //Drop frame
                av_frame_unref(_pFrame);
                return false;
            }
            if (usPTS > _sourceEndTime) {
                av_frame_unref(_pFrame);

                // adjust last frame duration and push to queue;
                if (_cacheLayer != nullptr && _targetEndTime - _cacheLayer->m_sllTimeStampUS > 0) {
                    _cacheLayer->m_sllDurationUS = _targetEndTime - _cacheLayer->m_sllTimeStampUS;
                    _layerList->push(_cacheLayer);
                    _cacheLayer = nullptr;
                }
                if (_cacheLayer != nullptr) {
                    CVLayerParam::release(_cacheLayer);
                    _cacheLayer = nullptr;
                }
                _outputFinish = true;
                return false;
            }
            return true;
        } else if (AVERROR(EAGAIN) == errorCode) {
            //more data
        } else if (AVERROR_EOF == errorCode) {
            // adjust last frame duration and push to queue;
            if (_cacheLayer != nullptr && _targetEndTime - _cacheLayer->m_sllTimeStampUS > 0) {
                _cacheLayer->m_sllDurationUS = _targetEndTime - _cacheLayer->m_sllTimeStampUS;
                _layerList->push(_cacheLayer);
                _cacheLayer = nullptr;
            }
            if (_cacheLayer != nullptr) {
                CVLayerParam::release(_cacheLayer);
                _cacheLayer = nullptr;
            }
            _outputFinish = true;
        } else {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "VideoDecoder._decodeFrame:Failed to receive frame!");
        }

        return false;
    }

//    static void savefirsth264(u8 *buffer, u32 size) {
//        FILE *fp = fopen("/storage/emulated/0/raw_n.h264", "wb");
//        if (fp == NULL) {
//            LOGE("open raw.pcm failed");
//            return;
//        }
//        fwrite(buffer, 1, size, fp);
//        fflush(fp);
//        fclose(fp);
//    }

    bool VideoDecoder::_readPacket() {
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
//
//        // to do
//        static bool save = false;
//        if (!save) {
//            savefirsth264(inputPacket.data, inputPacket.size);
//            save = true;
//        }

        if (AVERROR_EOF == errorCode) {
            _inputFinish = true;
        } else if (0 != errorCode) {
            av_packet_unref(&inputPacket);
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "VideoDecoder._readPacket:Failed to read frame!");
            return false;
        }

        errorCode = avcodec_send_packet(_pCodecContext, &inputPacket);
        if (0 > errorCode) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "VideoDecoder._readPacket:Failed to send packet!");
            av_packet_unref(&inputPacket);
            return false;
        }

        av_packet_unref(&inputPacket);
        return true;
    }

    void VideoDecoder::_cacheFrame() {
        CVLayerParam *pLayer = nullptr;

        do {
            if (!_pFrame || !_seekList.empty()) {
                break;
            }

            int64_t usPTS = av_rescale_q(
                    _pFrame->pts,
                    _pFormatContext->streams[_streamIndex]->time_base,
                    AV_TIME_BASE_Q);
            _prevFrameTime = usPTS;
            usPTS = usPTS - _sourceStartTime + _targetStartTime;
            int stride = _pFrame->linesize[0];
            int bufferSize = 0;
            bufferSize = stride * ALIGN(_pFrame->height, DEFAULT_ALIGN_DATA) * 3 / 2;

            pLayer = CVLayerParam::create();
            pLayer->resize(bufferSize);
            pLayer->m_sllTimeStampUS = usPTS;
            pLayer->m_sllDurationUS = s_defaultFrameDuration;
            pLayer->m_uSize = bufferSize;
            pLayer->m_uTrackId = _pTrack->getId();
            pLayer->m_eFormat = _pixelFormat;
            pLayer->m_uWidth = stride;
            pLayer->m_uHeight = ALIGN(_pFrame->height, DEFAULT_ALIGN_DATA);

            if (!_fillFrameData(_pFrame, pLayer)) {
                CVLayerParam::release(pLayer);
                pLayer = nullptr;
                break;
            }
            _fillAnimation(pLayer);
            float cropScaleX = _frameWidth == pLayer->m_uWidth ? 1.0f : (
                    _frameWidth * 1.0f / pLayer->m_uWidth - 0.01f);
            float cropScaleY = _frameHeight == pLayer->m_uHeight ? 1.0f : (
                    _frameHeight * 1.0f / pLayer->m_uHeight - 0.01f);
            pLayer->m_afUVCropScale[EM_DIRECT_X] *= cropScaleX;
            pLayer->m_afUVCropScale[EM_DIRECT_Y] *= cropScaleY;
            _fillEffect(pLayer);

            if (_cacheLayer) {
                _cacheLayer->m_sllDurationUS =
                        pLayer->m_sllTimeStampUS - _cacheLayer->m_sllTimeStampUS;
            }
        } while (false);

        if (_pFrame) {
            av_frame_unref(_pFrame);
        }
        if (_cacheLayer && pLayer) {
            if (_layerList->push(_cacheLayer)) {
                _cacheLayer = pLayer;
            } else {
                CVLayerParam::release(_cacheLayer);
                _cacheLayer = NULL;
                CVLayerParam::release(pLayer);
            }

        } else if (pLayer) {
            if (pLayer->m_sllTimeStampUS < (_targetEndTime - s_ptsBiasTime)) {
                _cacheLayer = pLayer;
            } else {
                //_layerList->push(pLayer);
                CVLayerParam::release(pLayer);
            }
        }
    }
//
//    static void savefirstYUV(AVFrame *srcFrame) {
//        int size = av_image_get_buffer_size(static_cast<AVPixelFormat>(srcFrame->format),
//                                            srcFrame->width,
//                                            srcFrame->height, 1);
//        uint8_t *buffer = static_cast<uint8_t *>(av_malloc(size));
//        if (!buffer) {
//            LOGE("Can not alloc buffer\n");
//            return;
//        }
//        int ret = av_image_copy_to_buffer(buffer, size,
//                                          (const uint8_t *const *) srcFrame->data,
//                                          (const int *) srcFrame->linesize,
//                                          static_cast<AVPixelFormat>(srcFrame->format),
//                                          srcFrame->width, srcFrame->height, 1);
//        if (ret < 0) {
//            LOGE("Can not copy image to buffer\n");
//            av_free(buffer);
//            return;
//        }
//
//        FILE *fp = fopen("/storage/emulated/0/yuv_r.yuv", "wb");
//        if (fp == NULL) {
//            LOGE("open yuv_r.yuv failed");
//            av_free(buffer);
//            return;
//        }
//        if (fwrite(buffer, 1, size, fp) < 0) {
//            LOGE("Failed to dump raw data.\n");
//            fclose(fp);
//            av_free(buffer);
//            return;
//        }
//        fflush(fp);
//        fclose(fp);
//        av_free(buffer);
//    }
//
//    static void savefirstI420(u8 *buffer, u32 size) {
//        FILE *fp = fopen("/storage/emulated/0/yuv_i420.yuv", "wb");
//        if (fp == NULL) {
//            LOGE("open yuv_i420.yuv failed");
//            return;
//        }
//        fwrite(buffer, 1, size, fp);
//        fflush(fp);
//        fclose(fp);
//    }

    bool VideoDecoder::_fillFrameData(AVFrame *srcFrame, CVLayerParam *dstLayer) {
        if (!srcFrame || !dstLayer) {
            return false;
        }
        // to do

//        static bool save = false;
//        if (!save) {
//            savefirstYUV(srcFrame);
//            save = true;
//        }

        int adjustHeight = ALIGN(_frameHeight, DEFAULT_ALIGN_DATA);
        if (adjustHeight == _frameHeight) {
            av_image_copy_to_buffer(
                    dstLayer->m_pbyPicture,
                    dstLayer->m_uSize,
                    (const uint8_t *const *) srcFrame->data,
                    srcFrame->linesize,
                    (AVPixelFormat) srcFrame->format,
                    srcFrame->linesize[0],
                    srcFrame->height,
                    1);
        } else {
            uint8_t *destData[4];
            int destLinesize[4];
            int errorCode = av_image_alloc(destData, destLinesize, srcFrame->linesize[0],
                                           adjustHeight,
                                           AV_PIX_FMT_YUV420P, 1);
            if (0 > errorCode) {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "VideoDecoder._fillFrameData:Failed to alloc image!");
                return false;
            }
            sws_scale(_formatConverter,
                      (const uint8_t *const *) srcFrame->data,
                      srcFrame->linesize, 0,
                      srcFrame->height,
                      destData,
                      destLinesize);
            av_image_copy_to_buffer(
                    dstLayer->m_pbyPicture,
                    dstLayer->m_uSize,
                    (const uint8_t *const *) destData,
                    destLinesize,
                    AV_PIX_FMT_YUV420P,
                    destLinesize[0],
                    adjustHeight,
                    1);

            av_freep(&destData[0]);
        }

        // to do

//        static bool savei420 = false;
//        if (!savei420) {
//            savefirstI420(dstLayer->m_pbyPicture,dstLayer->m_uSize);
//            savei420 = true;
//        }
        return true;
    }

    void VideoDecoder::_fillAnimation(CVLayerParam *dstLayer) {
        int aniCount = ((CTrack *) _pTrack)->getAnimationCount();
        for (int i = 0; i < aniCount; ++i) {
            CAnimation *pAnimation = ((CTrack *) _pTrack)->getAnimation(i);
            if (!pAnimation) {
                continue;
            }
            //Validate animation time.
            int64_t aniStartTime = dstLayer->m_sllTimeStampUS;
            aniStartTime -= (_targetStartTime + (pAnimation->m_sllStart * 1000));
            if (aniStartTime < 0) {
                continue;
            }
            int64_t aniEndTime = dstLayer->m_sllTimeStampUS;
            if (pAnimation->m_sllDuration < 0) {
                aniEndTime -= _targetEndTime;
            } else {
                aniEndTime -= (_targetStartTime +
                               (pAnimation->m_sllStart + pAnimation->m_sllDuration) * 1000);
            }

            if (aniEndTime > 0) {
                continue;
            }
            //
            aniStartTime /= 1000;
            // transform params
            dstLayer->m_afTranslate[EM_DIRECT_X] =
                    pAnimation->getVecX() * aniStartTime + pAnimation->m_fStartTransX;
            dstLayer->m_afTranslate[EM_DIRECT_Y] =
                    pAnimation->getVecY() * aniStartTime + pAnimation->m_fStartTransY;
            dstLayer->m_afTranslate[EM_DIRECT_Z] =
                    pAnimation->getVecZ() * aniStartTime + pAnimation->m_fStartTransZ;

            dstLayer->m_afRotate[EM_DIRECT_X] =
                    pAnimation->getVecDegreeX() * aniStartTime + pAnimation->m_fStartDegreeX;
            dstLayer->m_afRotate[EM_DIRECT_Y] =
                    pAnimation->getVecDegreeY() * aniStartTime + pAnimation->m_fStartDegreeY;
            dstLayer->m_afRotate[EM_DIRECT_Z] =
                    pAnimation->getVecDegreeZ() * aniStartTime + pAnimation->m_fStartDegreeZ;

            dstLayer->m_afScale[EM_DIRECT_X] =
                    pAnimation->getVecScaleX() * aniStartTime + pAnimation->m_fStartScaleX;
            dstLayer->m_afScale[EM_DIRECT_Y] =
                    pAnimation->getVecScaleY() * aniStartTime + pAnimation->m_fStartScaleY;
            dstLayer->m_afScale[EM_DIRECT_Z] =
                    pAnimation->getVecScaleZ() * aniStartTime + pAnimation->m_fStartScaleZ;

            // crop params
            dstLayer->m_afUVCropTranslate[EM_DIRECT_X] =
                    pAnimation->getCropVecX() * aniStartTime + pAnimation->m_fCropStartTransX;
            dstLayer->m_afUVCropTranslate[EM_DIRECT_Y] =
                    pAnimation->getCropVecY() * aniStartTime + pAnimation->m_fCropStartTransY;
            dstLayer->m_afUVCropTranslate[EM_DIRECT_Z] =
                    pAnimation->getCropVecZ() * aniStartTime + pAnimation->m_fCropStartTransZ;

            dstLayer->m_afUVCropRotate[EM_DIRECT_X] =
                    pAnimation->getCropVecDegreeX() * aniStartTime +
                    pAnimation->m_fCropStartDegreeX;
            dstLayer->m_afUVCropRotate[EM_DIRECT_Y] =
                    pAnimation->getCropVecDegreeY() * aniStartTime +
                    pAnimation->m_fCropStartDegreeY;
            dstLayer->m_afUVCropRotate[EM_DIRECT_Z] =
                    pAnimation->getCropVecDegreeZ() * aniStartTime +
                    pAnimation->m_fCropStartDegreeZ;

            dstLayer->m_afUVCropScale[EM_DIRECT_X] =
                    pAnimation->getCropVecScaleX() * aniStartTime + pAnimation->m_fCropStartScaleX;
            dstLayer->m_afUVCropScale[EM_DIRECT_Y] =
                    pAnimation->getCropVecScaleY() * aniStartTime + pAnimation->m_fCropStartScaleY;
            dstLayer->m_afUVCropScale[EM_DIRECT_Z] =
                    pAnimation->getCropVecScaleZ() * aniStartTime + pAnimation->m_fCropStartScaleZ;

            // color alpha
            dstLayer->m_fAlpha =
                    pAnimation->getVecAlpha() * aniStartTime + pAnimation->m_fStartAlpha;
        }
    }

    void VideoDecoder::_fillEffect(CVLayerParam *dstLayer) {
        int effectCount = ((CTrack *) _pTrack)->getEffectCount();
        for (int i = 0; i < effectCount; ++i) {
            CEffect *pEffect = ((CTrack *) _pTrack)->getEffect(i);
            if (!pEffect) {
                continue;
            }
            //Validate Effect time.
            int64_t effectStartTime = dstLayer->m_sllTimeStampUS;
            effectStartTime -= (_targetStartTime + (pEffect->getStart() * 1000));
            if (effectStartTime < 0) {
                continue;
            }
            int64_t effectEndTime = dstLayer->m_sllTimeStampUS;
            if (pEffect->getDuration() < 0) {
                effectEndTime -= _targetEndTime;
            } else {
                effectEndTime -= (_targetStartTime +
                                  (pEffect->getStart() + pEffect->getDuration()) * 1000);
            }

            if (effectEndTime > 0) {
                continue;
            }
            //
            CVFilterParam *pFilter = CVFilterParam::create();
            pFilter->reset();
            pEffect->getFilter(pFilter->m_pFilterSource, pFilter->m_eType);
            if (pFilter->m_pFilterSource) {
                dstLayer->m_vFilterParams.push_back(pFilter);
            } else {
                CVFilterParam::release(pFilter);
            }
        }
    }

    void VideoDecoder::_skipFrame(int64_t usTime) {
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

                if (framePTS > (usTime - s_ptsBiasTime) || framePTS > _sourceEndTime) {
                    _cacheFrame();
                    break;
                }
            } else if (AVERROR(EAGAIN) == errorCode) {
                //more data
            } else if (AVERROR_EOF == errorCode) {
                _outputFinish = true;
            } else {
                __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                    "VideoDecoder._skipFrame:Failed to receive frame!");
                break;
            }
        } while (true);
    }

    void VideoDecoder::_processCommand() {
        CAutoLock autoLock(_decoderLock);
        int64_t usTime = _seekList.back();
        usTime = usTime - _targetStartTime + _sourceStartTime;

        _flush();
        _seekTo(usTime, true);

        _seekList.clear();
    }

    void VideoDecoder::_seekTo(int64_t usTime, bool exactly) {
        if (usTime < 0) {
            //Log warning.
            usTime = 0;
        }
        if (usTime > _sourceEndTime) {
            __android_log_print(ANDROID_LOG_WARN, "PaomianTV",
                                "VideoDecoder._seekTo:seekTime %lld is invalid!", usTime);
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
                                "VideoDecoder._seekTo:Failed to seek frame!");
            return;
        }
        _inputFinish = false;
        _outputFinish = false;
        _prevFrameTime = -_minInterval - 1;

        if (exactly) {
            _skipFrame(usTime);
        }
    }

    void VideoDecoder::_flush() {
        _layerList->disable();
        _layerList->clear();
        _layerList->enable();
        if (_currentLayer) {
            CVLayerParam::release(_currentLayer);
            _currentLayer = nullptr;
        }
        if (_cacheLayer) {
            CVLayerParam::release(_cacheLayer);
            _cacheLayer = nullptr;
        }
    }

    bool VideoDecoder::_checkLayerTime(CVLayerParam *pLayer, int64_t usPTS) {
        if (!pLayer) {
            return false;
        }
        int64_t startTime = pLayer->m_sllTimeStampUS;
        int64_t endTime = pLayer->m_sllTimeStampUS + pLayer->m_sllDurationUS;
        if (startTime > usPTS) {
            return true;
        }
        if (endTime > usPTS) {
            return true;
        }
        return false;
    }
}