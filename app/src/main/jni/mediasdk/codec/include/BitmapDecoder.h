#ifndef MEDIAENGINE_BITMAPDECODER_H
#define MEDIAENGINE_BITMAPDECODER_H

#include "IDecoder.h"

namespace std
{
    class thread;
}

namespace paomiantv
{

    class ILock;
    class BitmapDecoder : public IDecoder
    {
    public:
        BitmapDecoder(ITrack*);
        virtual ~BitmapDecoder();

        virtual bool prepare() override;

        virtual void release() override;

        virtual void start() override;

        virtual void stop() override;

        virtual void pause() override;

        virtual void resume() override;

        virtual void seekTo(int64_t usTime) override;

        /**
         * 预留方法，暂未实现
         */
        virtual void flush() override;

        virtual CVLayerParam *getImageLayer(int64_t usPTS) override;

        virtual CAudio *getAudio(int64_t usPTS) override;

    public:
        
    protected:
        bool initialize();

        void reset();

        CVLayerParam *_createBitmapLayer(int64_t usPTS);

        void _setLayerParam(CVLayerParam *pLayer);

        virtual void _parseTime() override;

    protected:
        ILock *_decoderLock;

        uint32_t _width;
        uint32_t _height;

        bool _isPrepared;
    };

}

#endif //MEDIAENGINE_BITMAPDECODER_H
