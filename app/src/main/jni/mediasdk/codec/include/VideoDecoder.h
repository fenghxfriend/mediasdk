#ifndef MEDIAENGINE_VIDEOCODER_H_H
#define MEDIAENGINE_VIDEOCODER_H_H

#include "IDecoder.h"
#include <deque>
#include <enum.h>
#include <safequeue.h>

#ifdef __cplusplus
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}
#endif

namespace std
{
    class thread;
}

#ifdef __cplusplus
extern "C"
{
#ifndef SwsContext
typedef struct SwsContext SwsContext;
#endif
}
#endif

namespace paomiantv
{

    class ILock;
    class CSemaphore;

    class VideoDecoder : public IDecoder
    {
    public:
        VideoDecoder(ITrack *pTrack);

        virtual ~VideoDecoder();

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
        bool initialize();

        void reset();

        void _startDecoderThread();

        bool _prepare();

        bool _createDecoder();

        void _releaseDecoder();

        bool _decodeFrame();

        bool _readPacket();

        void _cacheFrame();

        bool _fillFrameData(AVFrame *srcFrame, CVLayerParam *dstLayer);

        void _fillAnimation(CVLayerParam *dstLayer);

        void _fillEffect(CVLayerParam *dstLayer);

        /**
         * 跳过指定时间点之前的帧
         * @param usTime 跳帧时间点，单位为毫秒
         */
        void _skipFrame(int64_t usTime);

        void _processCommand();

        void _seekTo(int64_t usTime, bool exactly = false);

        void _flush();

        bool _checkLayerTime(CVLayerParam *pLayer, int64_t usPTS);

    protected:
        ILock *_decoderLock;
        std::thread * _decoderThread;
        std::deque<int64_t> _seekList;
        CVLayerParam *_currentLayer;
        CVLayerParam *_cacheLayer;
        CSafeQueue<CVLayerParam> *_layerList;
        CSemaphore *_layerSemaphore;

        AVFormatContext * _pFormatContext;
        AVCodecContext *_pCodecContext;
        AVFrame *_pFrame;
        EMPixelFormat _pixelFormat;
        SwsContext *_formatConverter;

        /**
         * Last decoded frame time in microseconds.
         * For dropping frame.
         */
        int64_t _prevFrameTime;

        int _streamIndex;
        uint32_t _frameWidth;
        uint32_t _frameHeight;
        bool _inputFinish;
        bool _outputFinish;
    };

}

#endif //MEDIAENGINE_VIDEOCODER_H_H
