#ifndef MEDIAENGINE_AUDIOMULTIDECODER_H
#define MEDIAENGINE_AUDIOMULTIDECODER_H

#include "IDecoder.h"
#include <vector>

namespace paomiantv
{

    class AudioDecoder;
    class AudioMultiDecoder : public IDecoder
    {
    public:
        AudioMultiDecoder(ITrack *pTrack);
        virtual ~AudioMultiDecoder();

    public:
        virtual bool prepare() override;

        virtual void release() override;

        virtual void start() override;

        virtual void stop() override;

        virtual void pause() override;

        virtual void resume() override;

        virtual void seekTo(int64_t usTime) override;

        virtual bool isSeeking() override;

        /**
         * 预留方法，暂未实现
         */
        virtual void flush() override;

        virtual CVLayerParam *getImageLayer(int64_t usPTS) override;

        virtual CAudio *getAudio(int64_t usPTS) override;

    protected:
        /**
         * 创建子track的解码器
         * @param targetStartTime 目标起始时间，单位为微秒
         * @param srcTrack 源track实例
         */
        IDecoder *_createSubDecoder(int64_t targetStartTime, ITrack *srcTrack);

        virtual void _parseTime() override;

    protected:
        ILock *_decoderLock;
        std::vector<IDecoder*> _audioDecoders;
        size_t _currentIndex;

        uint8_t *_audioBufferPool;
        uint32_t _audioBufferCapacity;
        uint32_t _audioBufferSize;
        uint32_t _trunkSize;

        bool _isFinished;
    };

}

#endif //MEDIAENGINE_AUDIOMULTIDECODER_H
