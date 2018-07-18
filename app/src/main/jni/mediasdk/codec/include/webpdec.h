//
// Created by ASUS on 2018/3/28.
//

#ifndef MEDIAENGINE_OLD_WEBPDEC_H
#define MEDIAENGINE_OLD_WEBPDEC_H

#include <webp/demux.h>
#include <autolock.h>
#include <thread.h>
#include <constant.h>


#define COLOR_8888_ALPHA_MASK 0xff000000
namespace paomiantv {

    class CWebpDec {
    public:
        CWebpDec();

        virtual ~CWebpDec();

        int setDataSource(char *achPath);

        inline char *getSrc();

        inline int getFrameCount();

        void getFrame(int index, void **buffer, int *size);

        inline int getCanvasWidth();

        inline int getCanvasHeight();

        int getFrameDuration(int index);

    private:
        bool initDemux(const char *path);

        bool constructDependencyChain();

        static void *ThreadWrapper(void *pThis);

        int ThreadEntry();

        int drawFrame(int frameNr,
                       u32 *outputPtr, int outputPixelStride, int previousFrameNr);

        void initializeFrame(const WebPIterator &currIter, u32 *currBuffer,
                             int currStride, const WebPIterator &prevIter, const u32 *prevBuffer,
                             int prevStride);

        bool decodeFrame(const WebPIterator &currIter, u32 *currBuffer,
                         int currStride, const WebPIterator &prevIter, const u32 *prevBuffer,
                         int prevStride);

        ILock *m_pLock;
        CThread *m_pThread;
        BOOL32 m_bSuccess;
        BOOL32 m_bFinish;
        int m_nLoopCount;

        char m_achFilePath[MAX_LEN_FILE_PATH];
        WebPDecoderConfig m_tDecoderconfig;
        WebPData m_tData;
        WebPDemuxer *m_ptDmux;

        int m_nCanvasWidth, m_nCanvasHeight;
        int m_nFrameCount;
        u32 m_uBGColor;

        u32 *m_ptFrames;
        bool *m_bIsKeyFrames;
        int *m_nDurations;
        u32* m_pPreservedBuffer;

        bool initDecoder();
    };

    inline char *CWebpDec::getSrc() {
        return m_achFilePath;
    }

    inline int CWebpDec::getFrameCount() {
        return m_nFrameCount;
    }

    inline int CWebpDec::getCanvasHeight() {
        return m_nCanvasHeight;
    }

    inline int CWebpDec::getCanvasWidth() {
        return m_nCanvasWidth;
    }

}


#endif //MEDIAENGINE_OLD_WEBPDEC_H
