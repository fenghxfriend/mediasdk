#ifndef MEDIAENGINE_WEBPDECODER_H
#define MEDIAENGINE_WEBPDECODER_H

#include "IDecoder.h"
#include <webp/demux.h>
#include <autolock.h>
#include <thread.h>
#include <constant.h>
#include <track.h>
#include <safequeue.h>
#include <thread>

namespace paomiantv {
    typedef struct tagWebPFrame {
        uint32_t *_data;
        uint32_t _size;
        int _index;
        bool _isKey;
        uint32_t _width;
        uint32_t _height;
        int _pts;
        int _duration;
    public:
        tagWebPFrame() { memset(this, 0, sizeof(struct tagWebPFrame)); }

        ~tagWebPFrame() {
            if (_data != nullptr) {
                delete[] _data;
                _data = nullptr;
            }
        }
    } WebPFrame;

    class WebpDecoder : public IDecoder {
    public:
        WebpDecoder(ITrack *);

        virtual ~WebpDecoder();

    public:
        virtual bool prepare() override;

        virtual void release() override;

        virtual void start() override;

        virtual void stop() override;

        virtual void pause() override;

        virtual void resume() override;

        virtual void seekTo(int64_t usTime) override;

        virtual bool isSeeking() override {
            return !_seekList.empty();
        }

        /**
         * 预留方法，暂未实现
         */
        virtual void flush() override;

        virtual CVLayerParam *getImageLayer(int64_t usPTS) override;

        virtual CAudio *getAudio(int64_t usPTS) override;

    public:
        /**
         * 解码线程运行函数
         * 此方法为内部方法，使用者请勿调用
         */
        void run();

    protected:
        bool initDemux(const char *path);

        bool constructDependencyChain();

        void initializeFrame(const WebPIterator &currIter, uint32_t *currBuffer,
                             int currStride, const WebPIterator &prevIter,
                             const uint32_t *prevBuffer,
                             int prevStride);

        bool decodeFrame(const WebPIterator &currIter, uint32_t *currBuffer,
                         int currStride, const WebPIterator &prevIter, const uint32_t *prevBuffer,
                         int prevStride);

        bool initDecoder();

        void setLayerParams(CVLayerParam *layer, int64_t pts);

        void processCommand();

    protected:
        ILock *_decoderLock;
        std::thread *_decoderThread;
        std::deque<int64_t> _seekList;
        CVLayerParam *_currentLayer;
        CVLayerParam *_cacheLayer;
        CSafeQueue<CVLayerParam> *_layerList;
        CSemaphore *_layerSemaphore;

        /**
         * Last decoded frame time in microseconds.
         * For dropping frame.
         */
        int64_t _prevFrameTime;

        uint32_t _canvasWidth;
        uint32_t _canvasHeight;
        bool _inputFinish;
        bool _outputFinish;


        int _frameCount;
        uint32_t _bgColor;
        int _nextFrameToDecode;
        WebPDecoderConfig _decoderconfig;
        WebPData _webpData;
        WebPDemuxer *_wepDemux;
        uint32_t *_preservedBuffer;
        WebPFrame *_frameParams;
        WebPFrame *_pFrame;

        bool initialize();

        void reset();

        void _startDecoderThread();

        bool _prepare();

        void _releaseDecoder();

        bool _createDecoder();

        void _processCommand();

        bool _decodeFrame();

        void _cacheFrame();

        bool _checkLayerTime(CVLayerParam *pLayer, int64_t usPTS);

        void _fillEffect(CVLayerParam *dstLayer);

        void _fillAnimation(CVLayerParam *dstLayer);

        void _flush();

        void _seekTo(int64_t usTime, bool exactly = false);

        void _skipFrame(int64_t usTime, int key, int start);

        bool _seek_frame(int64_t usTime, int &keyIndex, int &targetIndex);
    };

}

#endif //MEDIAENGINE_WEBPDECODER_H
