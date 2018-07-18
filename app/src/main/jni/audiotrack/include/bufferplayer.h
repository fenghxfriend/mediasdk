//
// Created by ASUS on 2018/4/20.
//

#ifndef MEDIAENGINE_BUFFERPLAYER_H
#define MEDIAENGINE_BUFFERPLAYER_H

#include <SLES/OpenSLES_Android.h>
#include <safequeue.h>
#include <thread.h>
#include "aengine.h"
#include "sound.h"

namespace paomiantv {

    class CBufferPlayer {
    public:
        static CBufferPlayer *
        Create(EMSampleRate eSampleRate, u16 wEncoding, u32 uChannel, u32 uChannelMask);

        void destroy();

        BOOL32 sendBuffer(u8 *in, int size);

//        void start();

//        void stop();

        void play();

        void flush();

        void start();

        void stop();

        void pause();

        void resume();

        void setOnMessageCB(OnMessageCB cbOnMessage, void *cbDelegate);

    protected:
        void onPlayedMessage(s64 sllTimeStamp);

    private:
        CBufferPlayer(EMSampleRate eSampleRate, u16 wEncoding, u32 uChannel,
                      u32 uChannelMask);

        virtual ~CBufferPlayer();

        void release();

//
//        static void *ThreadWrapper(void *pThis);
//
//        void *run();

        // pointer and size of the next player buffer to enqueue, and number of remaining buffers
        static void BQPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

        BOOL32 prepare();


    protected:
        const EMSampleRate m_eSampleRate;
        const u32 m_uChannel;
        const u16 m_wEncoding;
        const u32 m_uChannelMask;

        ILock *m_pLock;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;

        CAudio *m_pLastFrame;

        CAEngine *m_pAEngine;
        SLObjectItf bqPlayerObject;
        SLPlayItf bqPlayerPlay;
        SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
        SLEffectSendItf bqPlayerEffectSend;
        SLMuteSoloItf bqPlayerMuteSolo;
        SLVolumeItf bqPlayerVolume;

        CSafeQueue<CAudio> *m_pAudios;

        u32 m_uHwBufferCapacity;// this can not modify,just init in configure

        u8 *m_pbyRemainderBuffer;
        u32 m_uHwRemainderBufferSize;

        u64 m_uHwBufferDurationUS;
        u64 m_uPlayTimeStampUS;


        OnMessageCB m_cbOnMessage;
        void *m_cbDelegate;

        void onSendEOSMessage();
    };
}


#endif //MEDIAENGINE_BUFFERPLAYER_H
