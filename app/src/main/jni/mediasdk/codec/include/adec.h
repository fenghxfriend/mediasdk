/*******************************************************************************
 *        Module: codec
 *          File:
 * Functionality: aac decoder
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-24  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef MEDIAENGINE_ADEC_H
#define MEDIAENGINE_ADEC_H

#include <track.h>
#include <thread.h>
#include <safequeue.h>
#include <sound.h>
#include <filtercomplex.h>
#include "dec.h"

namespace paomiantv {

    class CADec : public CDec {
    public:

        CADec(ITrack *const &pTrack);

        virtual ~CADec();

        virtual int prepare();

        virtual void release();

        virtual void start();

        virtual void stop();

        virtual void pause();

        virtual void resume();

        virtual BOOL32 getAudio(CAudio *&pAudio);

        BOOL32 getLayer(CVLayerParam *&pLayer);

        inline EMSampleRate getSampleRate() const;

        inline u64 getChannelLayout() const;

        inline AVSampleFormat getFormat() const;

        s16 getWeight();

        BOOL32 getRemainderBuffer(u8 *&out, u32 &size);

    private:

        static void *ThreadWrapper(void *pThis);

        int ThreadEntry();

        int adjustSeekTimestamp(s64 time, s64 &out);

    protected:
        virtual int decode();

        int decodeFrame();

        BOOL32 writeBuffer();

        BOOL32 processBuffer();

    protected:

        s64 m_sllCurrPlayUS;

        s64 m_sllStartPlayUS;
        s64 m_sllEndPlayUS;

        const AVSampleFormat m_eFormat = AV_SAMPLE_FMT_S16;
        const u64 m_ullChannelLayout = AV_CH_LAYOUT_STEREO;
        const EMSampleRate m_eSampleRate = EM_SAMPLE_RATE_44_1;

        u8 *m_pbyRemainderBuffer;
        u32 m_uRemainderBufferCapacity;
        u32 m_uOneFrameSize;
        u32 m_uRemainderBufferSize;


    private:
        CAFilterComplex *m_pAudioPreHandler;
        // audio source filter id
        u32 m_uSourceId;

        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;

        ILock *m_pLock;
        CThread *m_pThread;

        float m_fVolume;

        s64 m_sllEndCutUS;

        s32 m_nStreamIndex;
        AVFormatContext *m_ptAVFormatContext;
        AVCodecContext *m_ptAVCodecContext;
        AVFrame *m_ptAVFrame;
        AVFrame *m_ptRemainderFrame;
        u32 m_uRemainderSampleSize;


        CSafeQueue<CAudio> *m_pQueue;
    };

    inline EMSampleRate CADec::getSampleRate() const {
        return m_eSampleRate;
    }

    inline u64 CADec::getChannelLayout() const {
        return m_ullChannelLayout;
    }

    inline AVSampleFormat CADec::getFormat() const {
        return m_eFormat;
    }
}


#endif //MEDIAENGINE_ADEC_H
