//
// Created by ASUS on 2018/5/16.
//

#ifndef MEDIAENGINE_SILENCEDECODER_H
#define MEDIAENGINE_SILENCEDECODER_H


#include "IDecoder.h"

namespace std {
    class thread;
}

namespace paomiantv {

    class ILock;

    class SilenceDecoder : public IDecoder {
    public:
        SilenceDecoder(ITrack *);

        virtual ~SilenceDecoder();

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

    protected:
        bool initialize();

        void reset();

        CAudio *_createSilenceAudio();

        virtual void _parseTime() override;

    protected:
        ILock *_decoderLock;

        bool _isPrepared;
        uint32_t _trunkSize;
    };

}


#endif //MEDIAENGINE_SILENCEDECODER_H
