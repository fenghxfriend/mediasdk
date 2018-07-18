#include "WebpDecoder.h"
#include <webp/decode.h>
#include <autolog.h>
#include <unistd.h>
#include <enum.h>
#include <image.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "../../../webp/imageio/imageio_util.h"


namespace paomiantv {

#define COLOR_8888_ALPHA_MASK 0xff000000

    /**
   * PTS误差时间，单位为微秒
   */
    static const int64_t s_ptsBiasTime = 1 * 1000;

    // Returns true if the frame covers full canvas.
    static bool isFullFrame(const WebPIterator &frame, int canvasWidth, int canvasHeight) {
        return (frame.width == canvasWidth && frame.height == canvasHeight);
    }

    static bool willBeCleared(const WebPIterator &iter) {
        return iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND;
    }

    // Clear all pixels in a line to transparent.
    static void clearLine(u32 *dst, int width) {
        memset(dst, 0, width * sizeof(*dst));  // Note: Assumes TRANSPARENT == 0x0.
    }


    // Copy all pixels from 'src' to 'dst'.
    static void copyFrame(const u32 *src, int srcStride, u32 *dst, int dstStride,
                          int width, int height) {
        for (int y = 0; y < height; y++) {
            memcpy(dst, src, width * sizeof(*dst));
            src += srcStride;
            dst += dstStride;
        }
    }

    // return true if area of 'target' completely covers area of 'covered'
    static bool checkIfCover(const WebPIterator &target, const WebPIterator &covered) {
        const int covered_x_max = covered.x_offset + covered.width;
        const int target_x_max = target.x_offset + target.width;
        const int covered_y_max = covered.y_offset + covered.height;
        const int target_y_max = target.y_offset + target.height;
        return target.x_offset <= covered.x_offset
               && covered_x_max <= target_x_max
               && target.y_offset <= covered.y_offset
               && covered_y_max <= target_y_max;
    }

    // Returns true if the rectangle defined by 'frame' contains pixel (x, y).
    static bool FrameContainsPixel(const WebPIterator &frame, int x, int y) {
        const int left = frame.x_offset;
        const int right = left + frame.width;
        const int top = frame.y_offset;
        const int bottom = top + frame.height;
        return x >= left && x < right && y >= top && y < bottom;
    }

    WebpDecoder::WebpDecoder(ITrack *pTrack)
            : IDecoder(pTrack)
            , _decoderLock(nullptr)
            , _decoderThread(nullptr)
            , _seekList()
            , _layerList(nullptr)
            , _currentLayer(nullptr)
            , _cacheLayer(nullptr)
            , _layerSemaphore(nullptr)
            , _prevFrameTime(-_minInterval - 1)
            , _inputFinish(false)
            , _outputFinish(false)
            , _wepDemux(nullptr)
            , _pFrame(nullptr)
            , _frameParams(nullptr)
            , _preservedBuffer(nullptr)
            , _canvasWidth(0)
            , _canvasHeight(0)
            , _frameCount(0)
            , _bgColor(0)
            , _nextFrameToDecode(0) {
        memset(&_decoderconfig, 0, sizeof(WebPDecoderConfig));
        memset(&_webpData, 0, sizeof(WebPData));
        this->initialize();
    }

    WebpDecoder::~WebpDecoder() {
        release();
        reset();
    }

    bool WebpDecoder::initialize() {
        _decoderLock = new CLock();
        _layerList = new CSafeQueue<CVLayerParam>(_cacheCount);
        return true;
    }

    void WebpDecoder::reset() {
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

    bool WebpDecoder::prepare() {

        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderInvalid: {
                if (!_validateTrack(EM_TRACK_VIDEO, EM_SOURCE_WEBP)) {
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
                break;
        }

        //TODO:handle result
        return true;
    }

    static void _threadFunc(WebpDecoder *pDecoder) {
        if (pDecoder) {
            pDecoder->run();
        }
    }


    void WebpDecoder::_startDecoderThread() {
        _decoderThread = new std::thread(
                _threadFunc, this);
    }

    void WebpDecoder::run() {
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

    void WebpDecoder::_cacheFrame() {
        CVLayerParam *pLayer = nullptr;

        do {
            if (!_pFrame || !_seekList.empty()) {
                break;
            }
            int64_t usPTS = _pFrame->_pts * 1000;
            _prevFrameTime = usPTS;
            usPTS = usPTS - _sourceStartTime + _targetStartTime;

            uint32_t bufferSize = _canvasWidth * _canvasHeight * 4;
            pLayer = CVLayerParam::create();
            pLayer->resize(bufferSize);
            pLayer->m_sllTimeStampUS = usPTS;
            pLayer->m_sllDurationUS = 59 * 1000;
            pLayer->m_uTrackId = _pTrack->getId();
            pLayer->m_eFormat = EM_PIXEL_FORMAT_RGBA_8888;
            pLayer->m_uWidth = _pFrame->_width;
            pLayer->m_uHeight = _pFrame->_height;

            if (_pFrame->_data == nullptr || _pFrame->_size <= 0) {
                CVLayerParam::release(pLayer);
                pLayer = nullptr;
                break;
            }
            memcpy(pLayer->m_pbyPicture, _pFrame->_data, _pFrame->_size);
            pLayer->m_uSize = _pFrame->_size;

            _fillAnimation(pLayer);
            _fillEffect(pLayer);

            if (_cacheLayer) {
                _cacheLayer->m_sllDurationUS =
                        pLayer->m_sllTimeStampUS - _cacheLayer->m_sllTimeStampUS;
            }
        } while (false);

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

    bool WebpDecoder::_decodeFrame() {
        if (_outputFinish || _nextFrameToDecode >= _frameCount) {
            _outputFinish = true;
            return false;
        }

        // Find the first frame to be decoded.
        int start = MAX(_nextFrameToDecode - 1, 0);
        int earliestRequired = _nextFrameToDecode;
        while (earliestRequired > start) {
            if (_frameParams[earliestRequired]._isKey) {
                start = earliestRequired;
                break;
            }
            earliestRequired--;
        }

        WebPIterator currIter;
        WebPIterator prevIter;
        // Get frame number 'start - 1'.
        if (!WebPDemuxGetFrame(_wepDemux, start, &currIter)) {
            LOGE("Could not retrieve frame# %d", start - 1);
            return false;
        }

        // Use preserve buffer only if needed.
        u32 *prevBuffer = (_nextFrameToDecode == 0) ? _pFrame->_data : _preservedBuffer;
        int prevStride = _canvasWidth;
        u32 *currBuffer = _pFrame->_data;
        int currStride = _canvasWidth;

        for (int i = start; i <= _nextFrameToDecode; i++) {
            prevIter = currIter;
            // Get ith frame.
            if (!WebPDemuxGetFrame(_wepDemux, i + 1, &currIter)) {
                LOGE("Could not retrieve frame# %d", i);
                return false;
            }

            // We swap the prev/curr buffers as we go.
            u32 *tmpBuffer = prevBuffer;
            prevBuffer = currBuffer;
            currBuffer = tmpBuffer;

            int tmpStride = prevStride;
            prevStride = currStride;
            currStride = tmpStride;


            // Process this frame.
            initializeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer, prevStride);

            if (i == _nextFrameToDecode || !willBeCleared(currIter)) {
                if (!decodeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer,
                                 prevStride)) {
                    LOGE("Error decoding frame# %d", i);
                    return false;
                }
            }
        }

        if (_pFrame->_data != currBuffer) {
            copyFrame(currBuffer, currStride, _pFrame->_data, _canvasWidth, _canvasWidth,
                      _canvasHeight);
        }
        _pFrame->_duration = _frameParams[_nextFrameToDecode]._duration;
        _pFrame->_width = _canvasWidth;
        _pFrame->_height = _canvasHeight;
        _pFrame->_pts = _frameParams[_nextFrameToDecode]._pts;
        _pFrame->_size = _canvasHeight * _canvasWidth * 4;

        WebPDemuxReleaseIterator(&currIter);
        WebPDemuxReleaseIterator(&prevIter);

        if (_pFrame->_pts > _sourceEndTime) {
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
        } else {
            _nextFrameToDecode++;
            if (_nextFrameToDecode >= _frameCount) {
                _cacheFrame();
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
        }
        return true;
    }

    void WebpDecoder::_processCommand() {
        CAutoLock autoLock(_decoderLock);
        int64_t usTime = _seekList.back();
        usTime = usTime - _targetStartTime + _sourceStartTime;

        _flush();
        _seekTo(usTime, true);

        _seekList.clear();
    }


    void WebpDecoder::_flush() {
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


    void WebpDecoder::_seekTo(int64_t usTime, bool exactly) {
        if (usTime < 0) {
            //Log warning.
            usTime = 0;
        }
        if (usTime > _sourceEndTime) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "WebpDecoder._seekTo:seekTime %lld is invalid!", usTime);
            return;
        }

        int key = -1;
        int target = -1;
        if (!_seek_frame(usTime, key, target)) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "WebpDecoder._seekTo:Failed to seek frame!");
            return;
        }
        _inputFinish = false;
        _outputFinish = false;
        _prevFrameTime = -_minInterval - 1;
        _nextFrameToDecode = target;
        if (exactly) {
            _skipFrame(usTime, key, target);
        }
    }

    bool WebpDecoder::_seek_frame(int64_t usTime, int &keyIndex, int &targetIndex) {
        if (usTime <
            (_frameParams[_frameCount - 1]._pts + _frameParams[_frameCount - 1]._duration) * 1000 &&
            usTime >= 0) {
            int key = 0;
            for (int i = 0; i < _frameCount; i++) {
                if (_frameParams[i]._isKey) {
                    key = i;
                }
                if (usTime >= _frameParams[i]._pts * 1000 &&
                    usTime < (_frameParams[i]._pts + _frameParams[i]._duration) * 1000) {
                    keyIndex = key;
                    targetIndex = i;
                    return true;
                }
            }
        }
        return false;
    }

    void WebpDecoder::_skipFrame(int64_t usTime, int key, int target) {
        if (0 >= usTime || target < 0 || key < 0) {
            return;
        }
        if (_cacheLayer) {
            CVLayerParam::release(_cacheLayer);
        }
        int64_t usPTS = _frameParams[target]._pts * 1000;
        _prevFrameTime = usPTS;
        usPTS = usPTS - _sourceStartTime + _targetStartTime;

        uint32_t bufferSize = _canvasWidth * _canvasHeight * 4;
        _cacheLayer = CVLayerParam::create();
        _cacheLayer->resize(bufferSize);
        _cacheLayer->m_uSize = bufferSize;
        _cacheLayer->m_sllTimeStampUS = usPTS;
        _cacheLayer->m_sllDurationUS = _frameParams[target]._duration;
        _cacheLayer->m_uTrackId = _pTrack->getId();
        _cacheLayer->m_eFormat = EM_PIXEL_FORMAT_RGBA_8888;
        _cacheLayer->m_uWidth = _canvasWidth;
        _cacheLayer->m_uHeight = _canvasHeight;

        WebPIterator currIter;
        WebPIterator prevIter;
        // Get frame number 'start - 1'.
        if (!WebPDemuxGetFrame(_wepDemux, key, &currIter)) {
            LOGE("Could not retrieve frame# %d", key - 1);
            return;
        }

        // Use preserve buffer only if needed.
        u32 *prevBuffer = (target == 0) ? (u32 *) _cacheLayer->m_pbyPicture : _preservedBuffer;
        int prevStride = _canvasWidth;
        u32 *currBuffer = (u32 *) _cacheLayer->m_pbyPicture;
        int currStride = _canvasWidth;

        for (int i = key; i <= target; i++) {
            prevIter = currIter;
            // Get ith frame.
            if (!WebPDemuxGetFrame(_wepDemux, i + 1, &currIter)) {
                LOGE("Could not retrieve frame# %d", i);
                return;
            }

            // We swap the prev/curr buffers as we go.
            u32 *tmpBuffer = prevBuffer;
            prevBuffer = currBuffer;
            currBuffer = tmpBuffer;

            int tmpStride = prevStride;
            prevStride = currStride;
            currStride = tmpStride;


            // Process this frame.
            initializeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer, prevStride);

            if (i == target || !willBeCleared(currIter)) {
                if (!decodeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer,
                                 prevStride)) {
                    LOGE("Error decoding frame# %d", i);
                    return;
                }
            }
        }

        if ((u32 *) _cacheLayer->m_pbyPicture != currBuffer) {
            copyFrame(currBuffer, currStride, (u32 *) _cacheLayer->m_pbyPicture, _canvasWidth,
                      _canvasWidth,
                      _canvasHeight);
        }

        _nextFrameToDecode++;
        WebPDemuxReleaseIterator(&currIter);
        WebPDemuxReleaseIterator(&prevIter);
        _fillAnimation(_cacheLayer);
        _fillEffect(_cacheLayer);

        if (_cacheLayer->m_sllTimeStampUS >= (_targetEndTime - s_ptsBiasTime)) {
            //_layerList->push(pLayer);
            CVLayerParam::release(_cacheLayer);
            _cacheLayer = NULL;
        }
    }


    bool WebpDecoder::_prepare() {
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


    bool WebpDecoder::_createDecoder() {
        if (!_validateTrack(EM_TRACK_VIDEO, EM_SOURCE_WEBP)) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "WebpDecoder._createDecoder:Invalid track!");
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV",
                                "***---pTrack = %p, sourceType = %d",
                                _pTrack,
                                _pTrack ? _pTrack->getSourceType() : -1);
            return false;
        }

        std::string errorInfo = "";
        do {
            uint8_t *data = nullptr;
            uint32_t size = 0;
            _pTrack->getSourceData(data,size);

            if (!data || size == 0) {
                errorInfo = "Source of track is invalid!";
                break;
            }

            if (!initDemux((const char*)data)) {
                errorInfo = "Failed to init demux!";
                break;
            }

            if (!constructDependencyChain()) {
                errorInfo = "Failed to parse frames!";
                break;
            }

            if (!initDecoder()) {
                errorInfo = "Failed to init decoder!";
                break;
            }

            _pFrame = new WebPFrame;
            _pFrame->_data = new uint32_t[_canvasHeight * _canvasWidth];
            _pFrame->_size = _canvasHeight * _canvasWidth;
            _pFrame->_height = _canvasHeight;
        } while (false);

        if (!errorInfo.empty()) {
            _releaseDecoder();
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "WebpDecoder.createDecoder:%s",
                                errorInfo.c_str());
            return false;
        }

        return true;
    }


    void WebpDecoder::_releaseDecoder() {
        if (_wepDemux != nullptr) {
            WebPDemuxDelete(_wepDemux);
            _wepDemux = nullptr;
        }
        if (_webpData.bytes != nullptr) {
            delete[] _webpData.bytes;
        }
        if (_frameParams != nullptr) {
            delete[] _frameParams;
            _frameParams = nullptr;
        }
        if (_pFrame != nullptr) {
            delete _pFrame;
            _pFrame = nullptr;
        }

        if (_preservedBuffer != nullptr) {
            delete[] _preservedBuffer;
            _preservedBuffer = nullptr;
        }
        if (_pFrame) {
            delete[] _pFrame;
            _pFrame = nullptr;
        }

        _canvasWidth = 0;
        _canvasHeight = 0;
        _frameCount = 0;
    }

    void WebpDecoder::release() {
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
        if (_cacheLayer != nullptr) {
            CVLayerParam::release(_cacheLayer);
            _cacheLayer = nullptr;
        }
        if (_currentLayer != nullptr) {
            CVLayerParam::release(_currentLayer);
            _currentLayer = nullptr;
        }
    }

    void WebpDecoder::start() {
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

    void WebpDecoder::stop() {
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

    void WebpDecoder::pause() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderPlaying:
                _decoderState = EDecoderPaused;
                break;
            default:
                break;
        }
    }

    void WebpDecoder::resume() {
        CAutoLock autoLock(_decoderLock);
        switch (_decoderState) {
            case EDecoderPaused:
                _decoderState = EDecoderPlaying;
                break;
            default:
                break;
        }
    }

    void WebpDecoder::seekTo(int64_t usTime) {
        CAutoLock autoLock(_decoderLock);
        if (EDecoderInvalid == _decoderState) {
            return;
        }
        _layerList->disable();
        _seekList.push_back(usTime);
    }

    CVLayerParam *WebpDecoder::getImageLayer(int64_t usPTS) {
        if (EDecoderInvalid == _decoderState) {
            return nullptr;
        }

        if (usPTS < _targetStartTime && !_pTrack->isShowFirstFrame()) {
            return nullptr;
        }
        if (usPTS > _targetEndTime) {
            if (!_pTrack->isShowLastFrame()) {
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

    CAudio *WebpDecoder::getAudio(int64_t usPTS) {
        LOGW("No audio for webp decoder!");
        return nullptr;
    }

    void WebpDecoder::flush() {
        //Reserved.
        CAutoLock autoLock(_decoderLock);
    }

    void WebpDecoder::processCommand() {
        CAutoLock autoLock(_decoderLock);
        int64_t usTime = _seekList.back();
        usTime = usTime - _targetStartTime + _sourceStartTime;

        _flush();
        _seekTo(usTime, true);

        _seekList.clear();
    }

    bool WebpDecoder::initDemux(const char *path) {

        if (!ImgIoUtilReadFile(path,
                               &_webpData.bytes, &_webpData.size)) {
            LOGE("Get webp data from source file failed.\n");
            return false;
        }

        if (!WebPGetInfo(_webpData.bytes, _webpData.size, NULL, NULL)) {
            LOGE("Input file doesn't appear to be WebP format.\n");
            return false;
        }

        _wepDemux = WebPDemux(&_webpData);
        if (_wepDemux == NULL) {
            LOGE("Could not create demuxing object!\n");
            return false;
        }
        return true;
    }

    bool WebpDecoder::initDecoder() {
        if (!WebPInitDecoderConfig(&_decoderconfig)) {
            LOGE("Library version mismatch!\n");
            return false;
        }
        _decoderconfig.output.is_external_memory = 1;
        _decoderconfig.output.colorspace = MODE_rgbA;  // Pre-multiplied alpha mode.
        _preservedBuffer = new u32[_canvasHeight * _canvasWidth];
        return true;
    }

    void WebpDecoder::initializeFrame(const WebPIterator &currIter, u32 *currBuffer,
                                      int currStride, const WebPIterator &prevIter,
                                      const u32 *prevBuffer, int prevStride) {
        if (_frameParams[currIter.frame_num - 1]._isKey) {  // Clear canvas.
            for (int y = 0; y < _canvasHeight; y++) {
                u32 *dst = currBuffer + y * currStride;
                clearLine(dst, _canvasWidth);
            }
        } else {
            // Preserve previous frame as starting state of current frame.
            copyFrame(prevBuffer, prevStride, currBuffer, currStride, _canvasWidth,
                      _canvasHeight);

            // Dispose previous frame rectangle to Background if needed.
            bool prevFrameCompletelyCovered =
                    (!currIter.has_alpha || currIter.blend_method == WEBP_MUX_NO_BLEND) &&
                    checkIfCover(currIter, prevIter);
            if ((prevIter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
                !prevFrameCompletelyCovered) {
                u32 *dst = currBuffer + prevIter.x_offset + prevIter.y_offset * currStride;
                for (int j = 0; j < prevIter.height; j++) {
                    clearLine(dst, prevIter.width);
                    dst += currStride;
                }
            }
        }
    }

    bool WebpDecoder::decodeFrame(const WebPIterator &currIter, u32 *currBuffer,
                                  int currStride, const WebPIterator &prevIter,
                                  const u32 *prevBuffer,
                                  int prevStride) {
        u32 *dst = currBuffer + currIter.x_offset + currIter.y_offset * currStride;
        _decoderconfig.output.u.RGBA.rgba = (uint8_t *) dst;
        _decoderconfig.output.u.RGBA.stride = currStride * 4;
        _decoderconfig.output.u.RGBA.size =
                _decoderconfig.output.u.RGBA.stride * currIter.height;

        const WebPData &currFrame = currIter.fragment;
        if (WebPDecode(currFrame.bytes, currFrame.size, &_decoderconfig) != VP8_STATUS_OK) {
            return false;
        }
        // During the decoding of current frame, we may have set some pixels to be transparent
        // (i.e. alpha < 255). However, the value of each of these pixels should have been determined
        // by blending it against the value of that pixel in the previous frame if WEBP_MUX_BLEND was
        // specified. So, we correct these pixels based on disposal method of the previous frame and
        // the previous frame buffer.
        if (currIter.blend_method == WEBP_MUX_BLEND &&
            !_frameParams[currIter.frame_num - 1]._isKey) {
            if (prevIter.dispose_method == WEBP_MUX_DISPOSE_NONE) {
                for (int y = 0; y < currIter.height; y++) {
                    const int canvasY = currIter.y_offset + y;
                    for (int x = 0; x < currIter.width; x++) {
                        const int canvasX = currIter.x_offset + x;
                        u32 &currPixel = currBuffer[canvasY * currStride + canvasX];
                        // FIXME: Use alpha-blending when alpha is between 0 and 255.
                        if (!(currPixel & COLOR_8888_ALPHA_MASK)) {
                            const u32 prevPixel = prevBuffer[canvasY * prevStride + canvasX];
                            currPixel = prevPixel;
                        }
                    }
                }
            } else {  // prevIter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND
                // Need to restore transparent pixels to as they were just after frame initialization.
                // That is:
                //   * Transparent if it belongs to previous frame rectangle <-- This is a no-op.
                //   * Pixel in the previous canvas otherwise <-- Need to restore.
                for (int y = 0; y < currIter.height; y++) {
                    const int canvasY = currIter.y_offset + y;
                    for (int x = 0; x < currIter.width; x++) {
                        const int canvasX = currIter.x_offset + x;
                        u32 &currPixel = currBuffer[canvasY * currStride + canvasX];
                        // FIXME: Use alpha-blending when alpha is between 0 and 255.
                        if (!(currPixel & COLOR_8888_ALPHA_MASK)
                            && !FrameContainsPixel(prevIter, canvasX, canvasY)) {
                            const u32 prevPixel = prevBuffer[canvasY * prevStride + canvasX];
                            currPixel = prevPixel;
                        }
                    }
                }
            }
        }
        return true;
    }

    bool WebpDecoder::constructDependencyChain() {

        _canvasWidth = WebPDemuxGetI(_wepDemux, WEBP_FF_CANVAS_WIDTH);
        _canvasHeight = WebPDemuxGetI(_wepDemux, WEBP_FF_CANVAS_HEIGHT);
        _frameCount = WebPDemuxGetI(_wepDemux, WEBP_FF_FRAME_COUNT);

        _frameParams = new WebPFrame[_frameCount];

        WebPIterator prev;
        WebPIterator curr;

        // Note: WebPDemuxGetFrame() uses base-1 counting.
        if (!WebPDemuxGetFrame(_wepDemux, 1, &curr)) {
            LOGE("Could not retrieve frame# 0");
            return false;
        }

        _frameParams[0]._isKey = true;  // 0th frame is always a key frame.
        _frameParams[0]._pts = 0;
        _frameParams[0]._index = 1;
        _frameParams[0]._duration = curr.duration;

        for (size_t i = 1; i < _frameCount; i++) {
            prev = curr;
            if (!WebPDemuxGetFrame(_wepDemux, i + 1, &curr)) {// Get ith frame.
                LOGE("Could not retrieve frame# %d", i);
                return false;
            }

            if ((!curr.has_alpha || curr.blend_method == WEBP_MUX_NO_BLEND) &&
                isFullFrame(curr, _canvasWidth, _canvasHeight)) {
                _frameParams[i]._isKey = true;
            } else {
                _frameParams[i]._isKey = (prev.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
                                         (isFullFrame(prev, _canvasWidth, _canvasHeight) ||
                                          _frameParams[i - 1]._isKey);
            }
            _frameParams[i]._index = i + 1;
            _frameParams[i]._duration = curr.duration;
            _frameParams[i]._pts = _frameParams[i - 1]._pts + _frameParams[i - 1]._duration;
//            LOGE("webp frame%d, pts= %d, duration= %d!",i,_frameParams[i]._pts,_frameParams[i]._duration);
        }
        WebPDemuxReleaseIterator(&prev);
        WebPDemuxReleaseIterator(&curr);
        return true;
    }

    void WebpDecoder::setLayerParams(CVLayerParam *layer, s64 pts) {
        s32 asize = ((CTrack *) _pTrack)->getAnimationCount();
        for (int i = 0; i < asize; i++) {
            CAnimation *animation = ((CTrack *) _pTrack)->getAnimation(i);
            s64 offset = 0;
            if (animation != NULL &&
                (offset = (pts -
                           ((((CTrack *) _pTrack)->getCutStart() + animation->m_sllStart) *
                            1000))) >=
                0 && ((animation->m_sllDuration < 0) ? TRUE : (pts <=
                                                               ((((CTrack *) _pTrack)->getCutStart() +
                                                                 animation->m_sllStart +
                                                                 animation->m_sllDuration) *
                                                                1000)))) {
                offset /= 1000;
                // transform params
                layer->m_afTranslate[EM_DIRECT_X] =
                        animation->getVecX() * offset + animation->m_fStartTransX;
                layer->m_afTranslate[EM_DIRECT_Y] =
                        animation->getVecY() * offset + animation->m_fStartTransY;
                layer->m_afTranslate[EM_DIRECT_Z] =
                        animation->getVecZ() * offset + animation->m_fStartTransZ;

                layer->m_afRotate[EM_DIRECT_X] =
                        animation->getVecDegreeX() * offset + animation->m_fStartDegreeX;
                layer->m_afRotate[EM_DIRECT_Y] =
                        animation->getVecDegreeY() * offset + animation->m_fStartDegreeY;
                layer->m_afRotate[EM_DIRECT_Z] =
                        animation->getVecDegreeZ() * offset + animation->m_fStartDegreeZ;

                layer->m_afScale[EM_DIRECT_X] =
                        animation->getVecScaleX() * offset + animation->m_fStartScaleX;
                layer->m_afScale[EM_DIRECT_Y] =
                        animation->getVecScaleY() * offset + animation->m_fStartScaleY;
                layer->m_afScale[EM_DIRECT_Z] =
                        animation->getVecScaleZ() * offset + animation->m_fStartScaleZ;

                // crop params
                layer->m_afUVCropTranslate[EM_DIRECT_X] =
                        animation->getCropVecX() * offset + animation->m_fCropStartTransX;
                layer->m_afUVCropTranslate[EM_DIRECT_Y] =
                        animation->getCropVecY() * offset + animation->m_fCropStartTransY;
                layer->m_afUVCropTranslate[EM_DIRECT_Z] =
                        animation->getCropVecZ() * offset + animation->m_fCropStartTransZ;

                layer->m_afUVCropRotate[EM_DIRECT_X] =
                        animation->getCropVecDegreeX() * offset +
                        animation->m_fCropStartDegreeX;
                layer->m_afUVCropRotate[EM_DIRECT_Y] =
                        animation->getCropVecDegreeY() * offset +
                        animation->m_fCropStartDegreeY;
                layer->m_afUVCropRotate[EM_DIRECT_Z] =
                        animation->getCropVecDegreeZ() * offset +
                        animation->m_fCropStartDegreeZ;

                layer->m_afUVCropScale[EM_DIRECT_X] =
                        animation->getCropVecScaleX() * offset +
                        animation->m_fCropStartScaleX;
                layer->m_afUVCropScale[EM_DIRECT_Y] =
                        animation->getCropVecScaleY() * offset +
                        animation->m_fCropStartScaleY;
                layer->m_afUVCropScale[EM_DIRECT_Z] =
                        animation->getCropVecScaleZ() * offset +
                        animation->m_fCropStartScaleZ;

                // color alpha
                layer->m_fAlpha =
                        animation->getVecAlpha() * offset + animation->m_fStartAlpha;

            }
        }

        s32 esize = ((CTrack *) _pTrack)->getEffectCount();
        for (s32 i = 0; i < esize; i++) {
            CEffect *effect = ((CTrack *) _pTrack)->getEffect(i);
            if (effect != NULL &&
                (pts >=
                 ((((CTrack *) _pTrack)->getCutStart() + effect->getStart()) * 1000)) &&
                ((effect->getDuration() < 0) ? TRUE : (pts <=
                                                       ((((CTrack *) _pTrack)->getCutStart() +
                                                         effect->getStart() +
                                                         effect->getDuration()) *
                                                        1000)))) {
                CVFilterParam *filterParam = CVFilterParam::create();
                filterParam->reset();
                effect->getFilter(filterParam->m_pFilterSource, filterParam->m_eType);
                if (filterParam->m_pFilterSource != NULL) {
                    layer->m_vFilterParams.push_back(filterParam);
                } else {
                    CVFilterParam::release(filterParam);
                }
            }
        }
    }

    bool WebpDecoder::_checkLayerTime(CVLayerParam *pLayer, int64_t usPTS) {
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

    void WebpDecoder::_fillAnimation(CVLayerParam *dstLayer) {
        int aniCount = ((CTrack *) _pTrack)->getAnimationCount();
        for (int i = 0; i < aniCount; ++i) {
            CAnimation *pAnimation = ((CTrack *) _pTrack)->getAnimation(i);
            if (!pAnimation) {
                continue;
            }
            //Validate animation time.
            int64_t aniStartTime = dstLayer->m_sllTimeStampUS;
            aniStartTime -= (_sourceStartTime + pAnimation->m_sllStart) * 1000;
            if (aniStartTime < 0) {
                continue;
            }
            int aniEndTime = 0;
            if (pAnimation->m_sllDuration < 0) {
                aniEndTime = pAnimation->m_sllDuration;
            }
            aniEndTime =
                    (_sourceStartTime + pAnimation->m_sllStart + pAnimation->m_sllDuration) * 1000;
            if (aniEndTime > dstLayer->m_sllTimeStampUS) {
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

    void WebpDecoder::_fillEffect(CVLayerParam *dstLayer) {
        int effectCount = ((CTrack *) _pTrack)->getEffectCount();
        for (int i = 0; i < effectCount; ++i) {
            CEffect *pEffect = ((CTrack *) _pTrack)->getEffect(i);
            if (!pEffect) {
                continue;
            }
            //Validate Effect time.
            int64_t effectStartTime = _sourceStartTime + pEffect->getStart() * 1000;
            if (effectStartTime < dstLayer->m_sllTimeStampUS) {
                continue;
            }
            int64_t effectEndTime = 0;
            if (pEffect->getDuration() < 0) {
                effectEndTime = pEffect->getDuration();
            } else {
                effectEndTime = (((CTrack *) _pTrack)->getPlayStart() + pEffect->getStart() +
                                 pEffect->getDuration()) * 1000;
            }
            if (effectEndTime > dstLayer->m_sllTimeStampUS) {
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
}