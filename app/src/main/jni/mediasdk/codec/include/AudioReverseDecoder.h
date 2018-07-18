//
// Created by ASUS on 2018/6/1.
//

#ifndef MEDIAENGINE_AUDIOREVERSEDECODER_H
#define MEDIAENGINE_AUDIOREVERSEDECODER_H

#include <sound.h>
#include <safequeue.h>
#include "ReverseDecoder.h"

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
}
#endif

namespace paomiantv {
    class AudioReverseDecoder : public ReverseDecoder {
    public:
        AudioReverseDecoder(CDemuxer *demuxer);

        virtual ~AudioReverseDecoder();

        int prepare(s8 *tempPath);

        void release();

        virtual void stop();

        virtual void start();

        void getAudio(CAudio *&pAudio);

    private:
        virtual long run();

        int decode();

        CSafeQueue<CAudio> *m_pQueue;

        u32 m_uCurrSampleId;

        AVFormatContext *m_ptAVFormatContext;
        AVCodecContext *m_ptAVCodecContext;
        AVFrame *m_ptAVFrame;
        AVFrame *m_ptRemainderFrame;
        u32 m_uRemainderSampleSize;
        CAFilterComplex *m_pAudioPreHandler;
        u32 m_uSourceId;
        u32 m_uOneFrameSize;
        u32 m_uRemainderBufferCapacity;
        u8 *m_pbyRemainderBuffer;

        BOOL32 processBuffer();

        BOOL32 writeBuffer();

        u32 m_uRemainderBufferSize;
        BOOL32 m_bIsInputFinished;

        int decode_frame(AVFrame *&frame,  AVCodecContext *input_codec_context,
                     int *data_present, int *input_finish, int *finished);

        int reverseBuffer();

        int decodeFrame();
    };
}

#endif //MEDIAENGINE_AUDIOREVERSEDECODER_H
