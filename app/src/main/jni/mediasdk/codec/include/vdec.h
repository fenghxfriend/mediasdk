/*******************************************************************************
 *        Module: codec
 *          File:
 * Functionality: h264 decoder
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
#ifndef MEDIAENGINE_VDEC_H
#define MEDIAENGINE_VDEC_H


#include <track.h>
#include <safequeue.h>
#include <thread.h>
#include "dec.h"

#ifdef __cplusplus
extern "C"
{
#ifndef SwsContext
typedef struct SwsContext SwsContext;
#endif
}
#endif

namespace paomiantv {

    class CVDec : public CDec {
    public:

        CVDec(ITrack *const &pTrack);

        virtual ~CVDec();

        virtual int prepare();

        virtual void release();

        virtual void start();

        virtual void stop();

        virtual void pause();

        virtual void resume();

        virtual BOOL32 getLayer(CVLayerParam *&pLayer);

        virtual BOOL32 getAudio(CAudio *&pAudio);

        virtual BOOL32 getRemainderBuffer(u8 *&out, u32 &size);

    private:
        virtual int decode();

        int decodeFrame();

        static void *ThreadWrapper(void *pThis);

        int ThreadEntry();

        int adjustSeekTimestamp(s64 time, s64 &out);

        void findLayer(const s64 pts, CVLayerParam *&layer);

    protected:

        s64 m_sllCurrPlayUS;

        s64 m_sllStartPlayUS;
        s64 m_sllEndPlayUS;

        s64 m_sllLastPTSUS;


    private:


        ILock *m_pLock;
        CThread *m_pThread;

        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;

        s64 m_sllEndCutUS;

        // use by decoder
        CVLayerParam *m_pCacheLayer;

        // use by outside
        CVLayerParam *m_pLastLayer;

        CSafeQueue<CVLayerParam> *m_pQueue;

        AVFormatContext *m_ptAVFormatContext;
        AVCodecContext *m_ptAVCodecContext;
        AVFrame *m_ptAVFrame;
        s32 m_nStreamIndex;
        AVPixelFormat m_eDecoderOutputFmt;

        u32 m_uWidth;
        u32 m_uHeight;
        EMPixelFormat m_ePixFmt;
//        u32 m_uBufferSize;
        SwsContext *m_ptImgConvertContext;
    };
}


#endif //MEDIAENGINE_VDEC_H
