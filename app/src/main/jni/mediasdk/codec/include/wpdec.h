//
// Created by ASUS on 2018/3/28.
//

#ifndef MEDIAENGINE_VWPDEC_H
#define MEDIAENGINE_VWPDEC_H

#include <webp/demux.h>
#include <autolock.h>
#include <thread.h>
#include <constant.h>
#include <track.h>
#include <safequeue.h>
#include "dec.h"


#define COLOR_8888_ALPHA_MASK 0xff000000
namespace paomiantv {

    class CVWebPDec : public CDec {
    public:
        CVWebPDec(ITrack *const &pTrack);

        virtual ~CVWebPDec();

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
        bool initDemux(const char *path);

        bool constructDependencyChain();

        static void *ThreadWrapper(void *pThis);

        int ThreadEntry();

        int drawFrame();

        void initializeFrame(const WebPIterator &currIter, u32 *currBuffer,
                             int currStride, const WebPIterator &prevIter, const u32 *prevBuffer,
                             int prevStride);

        bool decodeFrame(const WebPIterator &currIter, u32 *currBuffer,
                         int currStride, const WebPIterator &prevIter, const u32 *prevBuffer,
                         int prevStride);

        void findLayer(const s64 pts, CVLayerParam *&layer);

        ILock *m_pLock;
        CThread *m_pThread;

        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;

        s64 m_sllEndCutUS;
        s64 m_sllCurrPlayUS;

        s64 m_sllStartPlayUS;
        s64 m_sllEndPlayUS;

        int m_nNextFrameToDecode;

        u32 m_uBufferSize;

        // use by outside
        CVLayerParam *m_pLastLayer;

        // use by decoder
        CVLayerParam *m_pCacheLayer;

        CSafeQueue<CVLayerParam> *m_pQueue;

        WebPDecoderConfig m_tDecoderconfig;
        WebPData m_tData;
        WebPDemuxer *m_ptDmux;

        u32 m_uFrameCount;
        u32 m_uWidth;
        u32 m_uHeight;
        u32 m_uBGColor;

        bool *m_bIsKeyFrames;
        u32 *m_pPreservedBuffer;

        bool initDecoder();

        int decode();


        bool getStartFrame();

        int m_nStartKeyFrameIndex;
        int m_nStartKeyFramePtsMS;
        int m_nStartFrameIndex;

        int m_nCurrFrameIndex;

        void setLayerParams(CVLayerParam *layer, s64 pts);

        s64 m_sllLastTimestampUS;
        s64 m_sllLastPTSUS;
        s64 m_sllCurrPTSUS;
    };
}


#endif //MEDIAENGINE_OLD_WEBPDEC_H
