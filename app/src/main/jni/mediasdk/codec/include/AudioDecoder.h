#ifndef MEDIAENGINE_AUDIODECODER_H
#define MEDIAENGINE_AUDIODECODER_H

#include "IDecoder.h"
#include <deque>
#include <enum.h>
#include <filtercomplex.h>
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

namespace paomiantv
{

    class ILock;
    class CSemaphore;

    class AudioDecoder : public IDecoder
    {
    public:
        AudioDecoder(ITrack*);
        virtual ~AudioDecoder();

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

        virtual uint8_t* getRemainderBuffer(uint32_t &size) override;
    
    public:
        public:
        /**
         * 解码线程运行函数
         * 此方法为内部方法，使用者请勿调用
         */
        void run();

    protected:
        bool initialize();

        void reset();

        bool _prepare();

        void _release();
        
        void _startDecoderThread();

        bool _initDecoder();

        bool _initFrames();

        bool _initAudioFilter();

        bool _initBuffer();

        void _initTime();

        bool _adjustSeekTimestamp(int64_t time, int64_t &out);

        bool _readPacket();
        
        bool _decodeFrame();
        
        bool _processBuffer();

        bool _writeBuffer();

        void _processCommand();

        void _seekTo(int64_t usTime);

        /**
         * 跳过指定时间点之前的帧
         * @param usTime 跳帧时间点，单位为毫秒
         */
        void _skipFrame(int64_t usTime);

        void _flush();

    public:
        static const AVSampleFormat s_sampleFormat = AV_SAMPLE_FMT_S16;
        static const uint64_t s_channelLayout = AV_CH_LAYOUT_STEREO;
        static const EMSampleRate s_sampleRate = EM_SAMPLE_RATE_44_1;

    protected:
        ILock *_decoderLock;
        std::thread *_decoderThread;
        std::deque<int64_t> _seekList;
        CSafeQueue<CAudio> *_audioList;
        CSemaphore *_audioSemaphore;

        AVFormatContext * _pFormatContext;
        AVCodecContext *_pCodecContext;
        AVFrame *_pFrame;
        AVFrame *_pRemainderFrame;
        uint32_t _remainderFrameSize;
        float m_fVolume;

        CAFilterComplex *_audioFilter;
        uint32_t _sourceID;
        uint8_t *_audioBufferPool;
        uint32_t _audioBufferCapacity;
        uint32_t _audioBufferSize;
        uint32_t _trunkSize;

        int _streamIndex;

        bool _inputFinish;
        
        bool _outputFinish;
    };

}

#endif //MEDIAENGINE_AUDIODECODER_H
