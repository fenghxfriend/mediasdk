//
// Created by ASUS on 2018/6/1.
//

#ifndef MEDIAENGINE_REVERSEDECODER_H
#define MEDIAENGINE_REVERSEDECODER_H

#include <thread.h>
#include <demuxer.h>

namespace paomiantv {
    class ReverseDecoder {
    public:
        ReverseDecoder(CDemuxer *demuxer);

        virtual ~ReverseDecoder();

    protected:
        virtual void start();

        virtual void stop();

        virtual long run()=0;

    protected:
        CDemuxer *m_pDemuxer;

        ILock *m_pLock;
        CThread *m_pThread;

        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;

        BOOL32 m_bIsFinished;

    private:
        static void *ThreadWrapper(void *pData);
    };
}
#endif //MEDIAENGINE_REVERSEDECODER_H
