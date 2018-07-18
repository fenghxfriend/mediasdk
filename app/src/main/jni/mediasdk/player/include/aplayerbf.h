#ifndef _PAOMIANTV_AUDIOPLAYERBQ_H
#define _PAOMIANTV_AUDIOPLAYERBQ_H

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "frame.h"
#include "audioplayer.h"
#include "../../../common/autolock.h"
#include "../../../common/safequeue.h"
#include "../../../common/thread.h"

namespace paomiantv {

    class CAPlayerBQ : public IBQAudioPlayer {
    public:
        CAPlayerBQ();

        virtual ~CAPlayerBQ();
        virtual void addToQueue(void *pframe);

        virtual void start();

        virtual void stop();

        virtual void popQueue(void *&pframe);

        BOOL32 isQueueEmpty();

        void setPlaying(BOOL32 isPlaying);

    public:
        void init(SLEngineItf engineEngine,SLObjectItf outputMixObject);

    private:
        CSafeQueue<CPCMFrame> *m_pQueue;
// buffer queue player interfaces
        SLObjectItf bqPlayerObject;
        SLPlayItf bqPlayerPlay;
        SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
        SLEffectSendItf bqPlayerEffectSend;
        SLMuteSoloItf bqPlayerMuteSolo;
        SLVolumeItf bqPlayerVolume;

        CPCMFrame* m_pFrame;
        s32 m_nOffset;
        s32 m_nSize;
        u8* m_pbyBuffer;

// pointer and size of the next player buffer to enqueue, and number of remaining buffers
        friend void BQPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

        CThread* m_pThread;
        ILock* m_pLock;
        BOOL32 m_bIsActived;
        BOOL32 m_bIsStop;
        static void* ThreadWrapper(void *pThis);
        int ThreadEntry();
        void destory();
    };

}
#endif //_PAOMIANTV_AUDIOPLAYERBQ_H
